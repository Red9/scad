/* 
 * File:   DatalogController.cpp
 * Author: clewis
 * 
 * Created on April 25, 2013, 4:48 PM
 */

#include "DatalogController.h"


extern Bluetooth * bluetooth;

DatalogController::DatalogController() {
    sd = NULL;
    currentFilename[0] = '\0';
    bufferFree = 999999; //Large number to start 

    killed = false;

    currentSerialLogState = false;
    currentSDLogState = false;

    sd = new SecureDigitalCard;

}

DatalogController::DatalogController(const DatalogController& orig) {
}

DatalogController::~DatalogController() {
    sd->Close();
    delete sd;
    sd = NULL;
}

/**
 * 
 * @param kPIN_SD_DO
 * @param kPIN_SD_CLK
 * @param kPIN_SD_DI
 * @param kPIN_SD_CS
 * @param lastFileNumber
 * @param unitNumber
 * @return true on success, false otherwise
 */
bool DatalogController::InitSD(int kPIN_SD_DO, int kPIN_SD_CLK,
        int kPIN_SD_DI, int kPIN_SD_CS,
        int lastFileNumber, int unitNumber
        ) {



    int mount = sd->Mount(kPIN_SD_DO, kPIN_SD_CLK,
            kPIN_SD_DI, kPIN_SD_CS);
    if (mount != 0) {
        //		debug->Put("Failed to mount SD card: %i\n\r", mount);
        return false;
    } else {

        return DatalogController::OpenFile(lastFileNumber, unitNumber) >= 0;
    }


}

/**
Scans the SD card and searches for filenames of the type "red9log#.csv",
where # is any size number. Actually, it uses the FILENAME and FILEEXT const
section variables for the filename. It starts scanning at filenumber 0, and
continues until it doesn't find a file of that number on the SD card. It
then opens the file for writing, and then this function returns.

@param sd
@param identifier The unit number to store in the first part of the filename.
@returns the current file number if successful, or a negative number if
 * unsuccessful. -1 is returned if all filenames are taken.
 */
bool DatalogController::OpenFile(int fileNumber, int identifier) {
    char buffer[12];
    //    char buff2[4];
    //int currentNumber = (lastFileNumber + 1) % 1000;
    //canonNumber refers to the last created file, so we need
    // to move to the next free one. This line is necessary
    // in case the file using the current canonNumber has been
    // deleted: we don't want to still create a file with
    // that number.


    //while (currentNumber != lastFileNumber) { //Loop until we have looped around.

    buffer[0] = 0;

    //This version is B###F###.EXT
    strcat(buffer, "B");
    strcat(buffer, Numbers::Dec(identifier));
    strcat(buffer, "F");
    strcat(buffer, Numbers::Dec(fileNumber));
    strcat(buffer, ".RNB");
    if(sd->Open(buffer, 'w') < 0){
        return false;
    }

    //Log the current filename
    strcpy(currentFilename, buffer);

    return true;
}

void DatalogController::SetClock(int year, int month, int day,
        int hour, int minute, int second) {
    if (sd != NULL) {
        sd->SetDate(year, month, day, hour, minute, second);
    }

}

int DatalogController::LogSequence(ConcurrentBuffer * sdBuffer) {
    volatile char * data;

    int data_size = 0;

    bufferFree = sdBuffer->GetFree();
    if (bufferFree != ConcurrentBuffer::GetkSize() - 1) {
        //If true, then some data to read.

        data_size = sdBuffer->Get(data);

        if (currentSerialLogState) {
            bluetooth->Put((char *) data, data_size);
        }
        if (currentSDLogState) {
            sd->Put((char *) data, data_size);
        }
    }

    return data_size;
}

void DatalogController::Server() {
    //WARNING: Must be called in it's own cog! (it has a cogstop at the end).

    killed = false; //Not killed on start.

    ConcurrentBuffer *serverBuffer = new ConcurrentBuffer();
    bufferFree = serverBuffer->GetkSize();

    int bytes_pulled = 0;

    while (!killed) {


        if (nextSDLogState == true) {
            currentSDLogState = true;
        } else if (nextSDLogState == false
                && currentSDLogState == true
                && bytes_pulled == 0) {
            //If we were logging but want to stop, and there is nothing left.

            currentSDLogState = false;
            sd->Close();
        }


        currentSerialLogState = nextSerialLogState;


        bytes_pulled = LogSequence(serverBuffer);



    }

    waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz

    //Finish up remaining bytes in buffer here.
    while (serverBuffer->GetFree() != serverBuffer->GetkSize() - 1) {
        LogSequence(serverBuffer);
    }

    waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz	

    sd->Close();
    delete serverBuffer;
}

char * DatalogController::GetCurrentFilename(void) {
    return currentFilename;
}

int DatalogController::GetBufferFree(void) {
    return bufferFree;
}

void DatalogController::KillServer(void) {
    killed = true;
}

void DatalogController::SetLogSerial(bool tlogSerial) {
    nextSerialLogState = tlogSerial;
}

void DatalogController::SetLogSD(bool tlogSD) {
    nextSDLogState = tlogSD;
}