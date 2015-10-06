#include "eventlog.h"

const char * EventLog::logString[] = {"\r\nOFF:   ", "\r\nFATAL: ", "\r\nERROR: ", "\r\nWARN:  ",
					  "\r\nINFO:  ", "\r\nDEBUG: ", "\r\nTRACE: ", "\r\nALL:   "};



EventLog::EventLog(int rxpin, int txpin, int baud){
	buffer[0] = '\0';
	LogDataStop();

	base.Start(rxpin, txpin, baud);
	SetLevel(kWarn);
}

EventLog::~EventLog(){
	base.Stop();
}



void EventLog::SetLevel(LogLevel newLevel){
	level = newLevel;
}


bool EventLog::LogInfo(LogLevel messageLevel, const char * message){
	if(ShouldLog(messageLevel)){
		Log(messageLevel, message);
		return true;
	}else{
		return false;
	}
}

bool EventLog::LogInfo(LogLevel messageLevel, const char * message, int a){
	strcpy(buffer, message);
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(a));
	return LogInfo(messageLevel, buffer);
}

bool EventLog::LogInfo(LogLevel messageLevel, const char * message, int a, int b){
	strcpy(buffer, message);
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(a));
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(b));
	return LogInfo(messageLevel, buffer);
}

bool EventLog::LogInfo(LogLevel messageLevel, const char * message, int a, int b, int c){
	strcpy(buffer, message);
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(a));
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(b));
	strcat(buffer, ", ");
	strcat(buffer, Numbers::Dec(c));
	return LogInfo(messageLevel, buffer);
}

bool EventLog::LogInfo(LogLevel messageLevel, const char * message, const char * a){
	strcpy(buffer, message);
	strcat(buffer, ", ");
	strcat(buffer, a);
	return LogInfo(messageLevel, buffer);
}


bool EventLog::LogDataStart(void){
	loggingData = true;
}

bool EventLog::LogDataStop(void){
	loggingData = false;
}

bool EventLog::LogData(char byte){
	if(loggingData){
		base.Put(byte);
		return true;	
	}else{
		return false;
	}
}


bool EventLog::ShouldLog(LogLevel messageLevel){
	if(level >= messageLevel && loggingData == false){
		return true;
	}else{
		return false;
	}
}

void EventLog::Log(LogLevel messageLevel, const char * message){

	base.Put(logString[messageLevel]);
	base.Put(message);
}





int EventLog::Get(char * buffer, char terminator){
	return base.Get(buffer, terminator);
}










