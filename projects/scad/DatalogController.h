/**
 * 
 * @author srlm (srlm@srlmproductions.com)
 */

#ifndef SRLM_PROPGCC_DATALOGCONTROLLER_H
#define	SRLM_PROPGCC_DATALOGCONTROLLER_H

#include "libpropeller/sd/sd.h"
#include "librednine/concurrent_buffer/concurrent_buffer.h"
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
    void InjectFile(const char * tfilename);
    void ListFilenamesOnDisk(void);



    void BlockUntilWaiting(void);

    bool getsdActive(void);

    bool getsdMounted(void);
    
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
    void ServerListFilenames(void);

    bool IsFileOnSD(const int fileNumber);
    void ComposeFileName(char * buffer, const int unit, const int canon);

    SD sd;
    ConcurrentBuffer sdBuffer;
    ConcurrentBuffer serialBuffer;

    volatile bool sdActive;


    int lastCanonNumber;
    int unitNumber;

    void ComposeElementHeader(char * data, const char elementIdentifier);
    
    /**
     * 
     * @param live
     * @param filename
     * @param filesize If filesize is unknown or isn't transmitted put -1 here.
     */
    void LogRElementBluetooth(const bool live, const char * filename, const int filesize);


    volatile bool sdMounted;
    volatile bool rootDirectoryIsOpen;

    void LogSequence(void);
    void LogSequenceSD(void);
    void LogSequenceSerial(void);
    

};

#endif	/* SRLM_PROPGCC_DATALOGCONTROLLER_H */

