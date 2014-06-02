#ifndef REDNINE_DATALOGCONTROLLER_H_
#define	REDNINE_DATALOGCONTROLLER_H_

#include "libpropeller/sd/sd.h"
#include "librednine/concurrent_buffer/concurrent_buffer.h"
#include "rovingbluetooth.h"
#include "Sensors.h"

extern Bluetooth bluetooth; //TODO(SRLM): This should be a parameter, not an extern.

#ifdef DEBUG_PORT
extern Serial debug;
#endif

class DatalogController {
public:

    struct Configuration {
        int unitNumber;
        int canonNumber;
        const char * sensorJSON;


        // Initial
        int year, month, day, hour, minute, second;
        int timestamp;


        // RFC822 format timezone
        char timezoneSign;
        int timezoneHours;
        int timezoneMinutes;


        const char * compileDate;
        const char * compileTime;

        int bitsPerTimestamp;

        int boardVersion;

        char filename[13];
    };

    ~DatalogController() {
        // TODO(SRLM): Stop logging (if logging).
    }

    bool Init(const int kPIN_SD_DO, const int kPIN_SD_CLK,
            const int kPIN_SD_DI, const int kPIN_SD_CS) {

        sdMounted = false;
        recording = kNO_RECORD;

        sd.Mount(kPIN_SD_DO, kPIN_SD_CLK, kPIN_SD_DI, kPIN_SD_CS);
        sdMounted = !sd.HasError();
        return sdMounted;
    }

    bool ListFiles(void) {
        bool successFlag = false;
        StopRecording();
        char filename[13];
        filename[0] = '\0';

        if (DiskReady() == true) {

            sd.OpenRootDirectory();


            int filesize, year, month, day, hour, minute, second;

            // TODO(SRLM): What if there is no output?
            while (sd.NextFile(filename, filesize, year, month, day, hour, minute, second) == true) {

                bluetooth.PutFormatted("%s %i %04i-%02i-%02iT%02i:%02i:%02i\r\n",
                        filename, filesize,
                        year, month, day, hour, minute, second);
            }
            successFlag = true;
        }
        return successFlag;
    }

    bool TransferFile(const char * filename) {
        StopRecording();
        bool successFlag = false;
        if (DiskReady() == true) {
            sd.ClearError();
            sd.Open(filename, 'r');
            if (sd.HasError()) {
                sd.ClearError();
            } else {
                int byte;
                //bluetooth.Put("{{{__[[[");

                //int sdStartCnt;
                //int bluetoothStartCnt;

                //int sdCntTotal = 0;
                //int bluetoothCntTotal = 0;

                //sdStartCnt = CNT;
                //debug.PutFormatted("%i|", sd.GetFilesize());
                bluetooth.PutFormatted("%i|", sd.GetFilesize());
                
                int bytesTransfered = 0;
                while ((byte = sd.Get()) != -1) {
                    //sdCntTotal += CNT - sdStartCnt;

                    //bluetoothStartCnt = CNT;
                    bluetooth.Put(byte);
                    bytesTransfered++;
                    //bluetoothCntTotal += CNT - bluetoothStartCnt;

                    if (bytesTransfered == 1024*10) {
                        bytesTransfered = 0;
                        int command = bluetooth.Get(2000);
                        if (command == -1) {
                            //Timeout!
                            break;
                        } else {
                            // No timeout, so go on.
                        }
                    }


                    //sdStartCnt = CNT;
                }
                //bluetooth.Put("]]]__}}}");
                //debug.Put("\r\nDone transferring");
#ifdef DEBUG_PORT
                //debug.PutFormatted("\r\nTransfer cycles; SD: %i; Bluetooth:%i", sdCntTotal, bluetoothCntTotal);
#endif

                successFlag = true;
            }
        }
        return successFlag;
    }

    void DeleteFile(const char * filename) {
        StopRecording();
        if (DiskReady() == true) {
            sd.ClearError();
            sd.Open(filename, 'd');
            sd.ClearError();
        }
    }

    void RecordFile(Configuration & config) {

        sd.SetDate(config.year, config.month, config.day, config.hour, config.minute, config.second);

        recording = kSTART_RECORD;

        if (ServerStartSD(config) == true) {
            recording = kRECORD;
            while (recording == kRECORD) {
                LogSequenceSD();
            }
            ServerStopSD();
        }
        recording = kNO_RECORD;
    }

    void StopRecording(void) {
        while (recording == kSTART_RECORD) {
            /* Do nothing */
        }

        if (recording == kRECORD) {
            recording = kFINISH_RECORDING;
        }

        while (recording != kNO_RECORD) {
            /* Do nothing */
        }
    }

    bool DiskReady(void) {
        return sdMounted;
    }

    bool IsRecording(void) {
        return recording != kNO_RECORD;
    }

    bool WaitUntilRecordingReady(void) {
        // Delay through the startup of recording
        while (recording == kSTART_RECORD) {
            /* Do nothing */
        }

        return recording == kRECORD;
    }


private:

    enum RecordingControl {
        kNO_RECORD, kSTART_RECORD, kRECORD, kFINISH_RECORDING
    };

    volatile RecordingControl recording;

    libpropeller::SD sd;
    ConcurrentBuffer sdBuffer;

    volatile bool sdMounted;

    bool OpenNewFile(Configuration & config) {
        char buffer[13];
        ComposeFileName(buffer, config.unitNumber, config.canonNumber);
        sd.Open(buffer, 'w');

        if (sd.HasError() == true) {
            sd.ClearError();
            sd.Close();
            return false;
        } else {
            strcpy(config.filename, buffer);
            OutputPreamble(config);
            return true;
        }
    }

    bool ServerStartSD(Configuration & config) {
        bool successFlag = false;
        if (DiskReady() == true) {
            int currentNumber = (config.canonNumber + 1) % 1000;
            while (currentNumber != config.canonNumber) {
                if (FindFile(config.unitNumber, currentNumber) == false) {
                    config.canonNumber = currentNumber;
                    successFlag = OpenNewFile(config);
                    break;
                }
                currentNumber = (currentNumber + 1) % 1000;
            }
            // TODO(SRLM): Do we really want to reset the tail? Shouldn't we take out a lock first?
            sdBuffer.ResetTail(); //Make sure that there are no unaccounted for bytes.
        }
        return successFlag;
    }

    void LogSequenceSD(void) {
        if (sdBuffer.GetFree() != ConcurrentBuffer::GetkSize()) {
            volatile char * data = NULL;
            int data_size = sdBuffer.Get(data);
            sd.Put((char *) data, data_size);
        }
    }

    void ServerStopSD(void) {
        //Finish up remaining bytes in buffer here.
        //TODO(SRLM): Make the log buffer cleanup have a timeout feature...
        if (ConcurrentBuffer::Lockset() == true) {
            LogSequenceSD();
            LogSequenceSD(); // Twice to make sure we've got everything.
            ConcurrentBuffer::Lockclear();
            waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz
        }
        sd.Close();
    }

    void ComposeFileName(char * buffer, const int unit, const int canon) {
        buffer[0] = 0;
        //This version is B###F###.EXT
        strcat(buffer, "B");
        strcat(buffer, libpropeller::Numbers::Dec(unit));
        strcat(buffer, "F");
        strcat(buffer, libpropeller::Numbers::Dec(canon));
        strcat(buffer, ".RNC");
    }

    bool FindFile(const int unitNumber, const int fileNumber) {

        char buffer[13];
        ComposeFileName(buffer, unitNumber, fileNumber);

        return FindFile(buffer);
    }

    bool FindFile(const char * filename) {
        sd.Open(filename, 'r');
        bool result = !sd.HasError();
        sd.ClearError();
        sd.Close();
        sd.ClearError();
        return result;
    }

    void OutputPreamble(Configuration & config) {

        static const char * const preambleJSON = R"PREAMBLE({
	"contentType":"RNC",
	"timestampFormat":{
		"frequency":%i,
		"bitsPerTimestamp":%i
	},
	"createTime":{
		"brokenTime":"%04i-%02i-%02iT%02i:%02i:%02i%c%02i%02i",
		"timestamp":"%08x"
	},
	"scadUnit":%i,
	"softwareVersion":"%s %s",
	"createName":"%s",
	"sensors":{
                %s
        }
        }
        )PREAMBLE";



        sd.PutFormatted(preambleJSON,
                CLKFREQ,
                config.bitsPerTimestamp,
                config.year, config.month, config.day, config.hour, config.minute, config.second,
                config.timezoneSign, config.timezoneHours, config.timezoneMinutes,
                config.timestamp,
                config.unitNumber,
                config.compileDate,
                config.compileTime,
                config.filename,
                config.sensorJSON);

        sd.Put("||||||||");
    }
};

#endif	// REDNINE_DATALOGCONTROLLER_H_

