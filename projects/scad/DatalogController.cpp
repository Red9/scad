#include "DatalogController.h"

extern Bluetooth * bluetooth; //TODO(SRLM): This should be a parameter, not an extren.

DatalogController::~DatalogController() {
    KillServer();
}

bool DatalogController::Init(int storedLastCanonNumber, int unitNumber, int kPIN_SD_DO, int kPIN_SD_CLK,
        int kPIN_SD_DI, int kPIN_SD_CS) {
    sdMounted = false;
    rootDirectoryIsOpen = false;

    this->lastCanonNumber = storedLastCanonNumber;
    this->unitNumber = unitNumber;

    sd.Mount(kPIN_SD_DO, kPIN_SD_CLK, kPIN_SD_DI, kPIN_SD_CS);

    if (sd.HasError() == true) {
        //		debug->Put("Failed to mount SD card: %i\n\r", sd.GetError());
        sdMounted = false;
    } else {
        sdMounted = true;
    }
    return sdMounted;
}

void DatalogController::ComposeFileName(char * buffer, const int unit, const int canon) {
    buffer[0] = 0;
    //This version is B###F###.EXT
    strcat(buffer, "B");
    strcat(buffer, Numbers::Dec(unit));
    strcat(buffer, "F");
    strcat(buffer, Numbers::Dec(canon));
    strcat(buffer, ".RNB");
}

bool DatalogController::OpenNewFile(void) {
    char buffer[13];
    ComposeFileName(buffer, unitNumber, lastCanonNumber);
    sd.Open(buffer, 'w');

    if (sd.HasError() == true) {
        sd.Close();
        return false;
    }
    return true;
}

bool DatalogController::IsFileOnSD(const int fileNumber) {
    char buffer[13];
    ComposeFileName(buffer, unitNumber, fileNumber);

    sd.Open(buffer, 'r');
    bool result = !sd.HasError();
    sd.Close();
    return result;
}

void DatalogController::SetClock(int year, int month, int day,
        int hour, int minute, int second) {
    sd.SetDate(year, month, day, hour, minute, second);
}

void DatalogController::LogSequenceSerial(void) {
    if (serialBuffer.GetFree() != ConcurrentBuffer::GetkSize()) {
        volatile char * data;
        int data_size = serialBuffer.Get(data);
        bluetooth->Put((char *) data, data_size);
    }
}

void DatalogController::LogSequenceSD(void) {


    if (sdActive == true && sdBuffer.GetFree() != ConcurrentBuffer::GetkSize()) {
        volatile char * data;
        int data_size = sdBuffer.Get(data);
        sd.Put((char *) data, data_size);
    }
}

void DatalogController::LogSequence() {
    LogSequenceSerial();
    LogSequenceSD();
}

void DatalogController::ServerStartSD(void) {
    if (sdActive == false) {
        sdActive = true;
        int currentNumber = (lastCanonNumber + 1) % 1000;
        while (currentNumber != lastCanonNumber) {
            if (IsFileOnSD(currentNumber) == false) {
                lastCanonNumber = currentNumber;
                OpenNewFile();
            }
            currentNumber = (currentNumber + 1) % 1000;
        }

        sdBuffer.ResetTail(); //Make sure that there are no unaccounted for bytes.

    }
}

void DatalogController::ServerStopSD(void) {
    if (sdActive == true) {
        //Finish up remaining bytes in buffer here.
        //TODO(SRLM): Make the log buffer cleanup have a timeout feature...

        if (ConcurrentBuffer::Lockset() == true) {
            LogSequenceSD();
            LogSequenceSD(); // Twice to make sure we've got everything.
            ConcurrentBuffer::Lockclear();
            waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz

        }
        sd.Close();
        sdActive = false;
    }
}

void DatalogController::ServerTransferFile(void) {

}

void DatalogController::Server(void) {

    while (true) {
        if (command == WAIT) {
            //Do Nothing (first in list for performance gain)
        } else if (command == LOG_SD) {
            ServerStartSD();
            command = WAIT;
        } else if (command == STOP_SD) {
            ServerStopSD();
            command = WAIT;
        } else if (command == KILL_SELF) {
            ServerStopSD();
            break;
        } else if (command == LIST_FILENAMES) {
            ServerOutputFilenames();
            command = WAIT;
        } else if (command == TRANSFER_FILE) {
            ServerStopSD();
            ServerTransferFile();
            command = WAIT;
        }

        LogSequence();
    }


    //sd.Unmount(); //TODO(SRLM): Should I have this here?
    command = WAIT;
    //TODO(SRLM): Finish up last bytes here

}

void DatalogController::GetCurrentFilename(char * filename) {
    return ComposeFileName(filename, unitNumber, lastCanonNumber);
}

void DatalogController::KillServer(void) {
    this->command = KILL_SELF;
}

void DatalogController::StartSD(void) {
    this->command = LOG_SD;
}

void DatalogController::StopSD(void) {
    this->command = STOP_SD;
}

void DatalogController::ListFilenamesOnDisk(void){
    this->command = LIST_FILENAMES;
}

void DatalogController::BlockUntilWaiting(void) {
    while (this->command != WAIT) {
        waitcnt(CLKFREQ / 1000000 + CNT);
    }
}

void DatalogController::TransferFile(const char * tfilename) {
    strcpy((char *) this->transferFilename, tfilename );
    this->command = TRANSFER_FILE;
}

int DatalogController::GetLastFileNumber(void) {
    return lastCanonNumber;
}

void DatalogController::ServerOutputFilenames(void) {
    char filename[13];
    filename[0] = '\0';

    if (sdMounted == true && sdActive == false) {

        if (ConcurrentBuffer::Lockset() == true) {
            LogSequenceSerial();
            LogSequenceSerial();
            ConcurrentBuffer::Lockclear();
        }

        //TODO(SRLM): output the element identifier and CNT here.
        bluetooth->Put("S\0\0\0\0");

        sd.OpenRootDirectory();

        bool firstTime = true;

        while (sd.NextFile(filename) == true) {
            if (firstTime == false) {
                bluetooth->Put(", ");
            }
            bluetooth->Put(filename);
            firstTime = false;
        }
        bluetooth->Put('\0');

    }
}