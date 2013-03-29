#include "pib.h"

void PIB::_3x2(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		unsigned int a, unsigned int b, unsigned int c){
	char data[13]; //TODO(SRLM): Does this need to be 13 bytes?
	data[0] = identifier;
	
//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
// Two byte mode
	data[6] = (a & 0xFF00)     >> 8;
	data[5] = (a & 0xFF)       >> 0;
	
	data[8] = (b & 0xFF00)     >> 8;
	data[7] = (b & 0xFF)       >> 0;
	
	data[10]= (c & 0xFF00)     >> 8;
	data[ 9]= (c & 0xFF)       >> 0;

	while(buffer->Put(data, 11) == false){}			
}

void PIB::_2x4(ConcurrentBuffer * buffer, const char identifier,
                   const unsigned int cnt, const int a, const int b){
	char data[13];
	data[0] = identifier;
	
	//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
	data[8] = (a & 0xFF000000) >> 24;
	data[7] = (a & 0xFF0000)   >> 16;
	data[6] = (a & 0xFF00)     >> 8;
	data[5] = (a & 0xFF)       >> 0;
	
	data[12] = (b & 0xFF000000) >> 24;
	data[11] = (b & 0xFF0000)   >> 16;
	data[10] = (b & 0xFF00)     >> 8;
	data[9]  = (b & 0xFF)       >> 0;
	
	while(buffer->Put(data, 13) == false){}
}

void PIB::_3x4(ConcurrentBuffer * buffer, const char identifier,
                   const unsigned int cnt,
                   const int a, const int b, const int c){
	char data[17];
	data[0] = identifier;
	
	//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
	data[8] = (a & 0xFF000000) >> 24;
	data[7] = (a & 0xFF0000)   >> 16;
	data[6] = (a & 0xFF00)     >> 8;
	data[5] = (a & 0xFF)       >> 0;
	
	data[12] = (b & 0xFF000000) >> 24;
	data[11] = (b & 0xFF0000)   >> 16;
	data[10] = (b & 0xFF00)     >> 8;
	data[9]  = (b & 0xFF)       >> 0;
	
	data[16] = (c & 0xFF000000) >> 24;
	data[15] = (c & 0xFF0000)   >> 16;
	data[14] = (c & 0xFF00)     >> 8;
	data[13] = (c & 0xFF)       >> 0;
	
	while(buffer->Put(data, 17) == false){}
}



void PIB::_string(ConcurrentBuffer * buffer, char identifier, unsigned int cnt,
		const char * string, char terminator){
	char data[5];
	data[0] = identifier;
	
//Little endian
	data[4] = (cnt & 0xFF000000) >> 24;
	data[3] = (cnt & 0xFF0000)   >> 16;
	data[2] = (cnt & 0xFF00)     >> 8;
	data[1] = (cnt & 0xFF)       >> 0;
	
	while(buffer->Put(data, 5, string, terminator) == false){}
	
}
