/**
 * 
 * @author srlm (srlm@srlmproductions.com)
 */

#ifndef SRLM_PROPGCC_DATALOGCONTROLLER_H
#define	SRLM_PROPGCC_DATALOGCONTROLLER_H

#include "securedigitalcard.h"
#include "concurrentbuffer.h"
#include "rovingbluetooth.h"

class DatalogController {
public:
    ~DatalogController();

    bool Init(int storedLastCanonNumber, int unitNumber, int kPIN_SD_DO, int kPIN_SD_CLK, int kPIN_SD_DI, int kPIN_SD_CS);

    void SetClock(int year, int month, int day,
            int hour, int minute, int second);

    void Server(void);
    void KillServer(void);

    void GetCurrentFilename(char * filename);
    int GetLastFileNumber(void);


    void StartSD(void);
    void StopSD(void);
    void TransferFile(const char * tfilename);
    void ListFilenamesOnDisk(void);



    void BlockUntilWaiting(void);




private:
    bool OpenNewFile(void);

    enum COMMAND_LIST {
        WAIT, LOG_SD, STOP_SD, TRANSFER_FILE, LIST_FILENAMES, KILL_SELF
    };
    volatile COMMAND_LIST command;
    volatile char transferFilename[13];


    void ServerStopSD(void);
    void ServerTransferFile(void);
    void ServerStartSD(void);
    void ServerOutputFilenames(void);

    bool IsFileOnSD(const int fileNumber);
    void ComposeFileName(char * buffer, const int unit, const int canon);

    SecureDigitalCard sd;
    ConcurrentBuffer sdBuffer;
    ConcurrentBuffer serialBuffer;

    bool sdActive;


    int lastCanonNumber;
    int unitNumber;



    volatile bool sdMounted;
    volatile bool rootDirectoryIsOpen;

    void LogSequence(void);
    void LogSequenceSD(void);
    void LogSequenceSerial(void);
    

};

#endif	/* SRLM_PROPGCC_DATALOGCONTROLLER_H */

