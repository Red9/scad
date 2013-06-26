#include "DatalogController.h"

extern Bluetooth * bluetooth;


 extern Serial * debug;

DatalogController::~DatalogController() {
    Stop();
}

void DatalogController::Stop(void) {
    sd.Unmount();
    sdMounted = false;
}

bool DatalogController::Start(int kPIN_SD_DO, int kPIN_SD_CLK,
        int kPIN_SD_DI, int kPIN_SD_CS) {
    sdMounted = false;
    rootDirectoryIsOpen = false;
    currentFilename[0] = '\0';
    bufferFree = 999999; //Large number to start 
    
    transmitFile = false;

    killed = false;

    currentSerialLogState = false;
    currentSDLogState = false;

    sd.Mount(kPIN_SD_DO, kPIN_SD_CLK, kPIN_SD_DI, kPIN_SD_CS);

    
    if (sd.HasError()) {
        debug->PutFormatted("\r\nMount: sd.HasError() == true, SD Error: %i", sd.GetError());
    }
    
    if (sd.HasError() == true) {
        //		debug->Put("Failed to mount SD card: %i\n\r", sd.GetError());
        sdMounted = false;
    } else {
        sdMounted = true;
    }
    return sdMounted;
}

/**
 * @param lastFileNumber
 * @param unitNumber
 * @return true on success, false otherwise
 */
bool DatalogController::InitSD(int lastFileNumber, int unitNumber) {
    return DatalogController::OpenFile(lastFileNumber, unitNumber) >= 0;
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
    buffer[0] = 0;
    //This version is B###F###.EXT
    strcat(buffer, "B");
    strcat(buffer, Numbers::Dec(identifier));
    strcat(buffer, "F");
    strcat(buffer, Numbers::Dec(fileNumber));
    strcat(buffer, ".RNB");
    sd.Open(buffer, 'w');
    
    if (sd.HasError()) {
        debug->PutFormatted("\r\nOpen File: sd.HasError() == true, SD Error: %i", sd.GetError());
    }
    
    if (sd.HasError() == true) {
        return false;
    }
    //Save the current filename
    strcpy(currentFilename, buffer);
    return true;
}

void DatalogController::SetClock(int year, int month, int day,
        int hour, int minute, int second) {
    sd.SetDate(year, month, day, hour, minute, second);
}

int DatalogController::LogSequence(ConcurrentBuffer * sdBuffer) {
    volatile char * data;

    int data_size = 0;

    bufferFree = sdBuffer->GetFree();
    if (bufferFree != ConcurrentBuffer::GetkSize()) {
        //If true, then some data to read.

        data_size = sdBuffer->Get(data);

        if (currentSerialLogState) {
            bluetooth->Put((char *) data, data_size);
        }
        if (currentSDLogState) {
            sd.Put((char *) data, data_size);
        }
    }
    
    return data_size;
}

void DatalogController::Server() {

    killed = false; //Not killed on start.

    ConcurrentBuffer serverBuffer;

    int bytes_pulled = 0;

    while (!killed) {
        if (nextSDLogState == true) {
            currentSDLogState = true;
        } else if (nextSDLogState == false
                && currentSDLogState == true
                && bytes_pulled == 0) {
            //If we were logging but want to stop, and there is nothing left.

            currentSDLogState = false;
            sd.Close();
        }
        currentSerialLogState = nextSerialLogState;
        bytes_pulled = LogSequence(&serverBuffer);
    }

    waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz

    //Finish up remaining bytes in buffer here.
    while (serverBuffer.GetFree() != serverBuffer.GetkSize()) {
        LogSequence(&serverBuffer);
    }

    waitcnt(CLKFREQ / 100 + CNT); //10 ms @80MHz	
    sd.Close();
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

void DatalogController::TransmitFile(char* filename){
    strcpy(filename, filenameToTransmit);
    transmitFile = true;
}

bool DatalogController::GetNextFilenameOnDisk(char * filenameOutput) {
   

    if (sd.HasError()) {
        debug->PutFormatted("\r\nGetNextFilenameOnDisk: sd.HasError() == true, SD Error: %i", sd.GetError());
    }

    if (sdMounted == true) {
        //debug->Put("\r\nsdMounted == true");
        if (rootDirectoryIsOpen == false) {
            //debug->Put("\r\nrootDirectoryIsOpen == false");
            sd.OpenRootDirectory();
            rootDirectoryIsOpen = true;
        }

        if (sd.NextFile(filenameOutput) == false) {
            //debug->Put("\r\nsd.NextFile(filenameOutput) == false");
            rootDirectoryIsOpen = false;
            return false;
        } else {
            //debug->Put("\r\nsd.NextFile(filenameOutput) == true");
            return true;
        }


    } else {
        return false;
    }
}