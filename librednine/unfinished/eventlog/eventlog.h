/*
SRLM February 22, 2013
- I wrote this class to be a debugging interface to the SCAD project.
Unforutnately, it doesn't mesh very well. That project needs to be able to
stream binary data out on a port, and still be able to debug messages. So, I've
decided to scrap this class and get back to basics with another design.

This class seems to mostly work. In particular, the LogInfo(*) functions work.
The LogData functions should work as well, but I don't really have any use for
those (notice how it's LogData(byte)? That's to accomodate the binary data...)




*/


#ifndef SRLM_PROPGCC_EVENTLOG_H_
#define SRLM_PROPGCC_EVENTLOG_H_


#include "serial.h"
#include "numbers.h"

class EventLog{


public:

enum LogLevel{kOff, kFatal, kError, kWarn, kInfo, kDebug, kTrace, kAll};




EventLog(int rxpin = 31, int txpin = 30, int baud = 460800);
~EventLog();



bool LogInfo(LogLevel messageLevel, const char * message);
bool LogInfo(LogLevel messageLevel, const char * message, int a);
bool LogInfo(LogLevel messageLevel, const char * message, int a, int b);
bool LogInfo(LogLevel messageLevel, const char * message, int a, int b, int c);
bool LogInfo(LogLevel messageLevel, const char * message, const char * a);


void SetLevel(LogLevel newlevel);

bool LogDataStart(void);
bool LogDataStop(void);
bool LogData(char byte);


int Get(char * buffer, char terminator);

private:


LogLevel level;
Serial base;


static const char * logString[];
bool loggingData;

bool ShouldLog(LogLevel level);
void Log(LogLevel messageLevel, const char * message);


static const int kBufferSize = 200;
char buffer[kBufferSize];


};


#endif // SRLM_PROPGCC_EVENTLOG_H_
