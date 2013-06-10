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
    
    void Start(void);
    void Stop(void);
    
    bool InitSD(int kPIN_SD_DO, int kPIN_SD_CLK,
            int kPIN_SD_DI, int kPIN_SD_CS,
            int lastFileNumber, int unitNumber);
    void Server(void);
    void KillServer(void);
    bool OpenFile(int fileNumber, int identifier);
    
    char * GetCurrentFilename(void);
    int GetBufferFree(void);
    
    
    /**
     @Warning: Logging stops as soon as this is set, so make sure that
    there isn't anything left in the buffer!
    */
    void SetLogSerial(bool logSerial);
    /**
     @Warning: Logging stops as soon as this is set, so make sure that
    there isn't anything left in the buffer!
    */
    void SetLogSD(bool logSD);
    
    void SetClock(int year, int month, int day,
                       int hour, int minute, int second);

private:
    SecureDigitalCard sd;
    char currentFilename[13];
    int bufferFree;
    
    volatile bool killed;
    
    
    
    bool currentSerialLogState;
    bool currentSDLogState;
    volatile bool nextSerialLogState;
    volatile bool nextSDLogState;
    
    
    /**
     * 
     * @param sdBuffer
     * @returns the number of bytes pulled from the buffer.
     */
    int LogSequence(ConcurrentBuffer * sdBuffer);
    
};

#endif	/* SRLM_PROPGCC_DATALOGCONTROLLER_H */

