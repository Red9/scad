#ifndef i2c_Class_Defined__
#define i2c_Class_Defined__

#include <stdio.h>

#ifdef UNIT_TEST

/*

Notes:
- This is a MOCK object to allow for unit testing of the L3GD20 object.

*/
class i2c {
public:

 
  static const int kAck = 1;
  static const int kNak = 0;
  
  char putStack[100];
  int putStackTop;
  int ack;
  
  char xyz[6];
  
  
  int Initialize(int SCLPin, int SDAPin)
  {
  	putStackTop = -1;
  	ack = kAck;
  	
  }
 
  int Ping(unsigned char device)
  {
  	return ack;
  }

  unsigned char Put(unsigned char device, unsigned char address, char byte)
  {
  	
  	putStack[++putStackTop] = byte;
  	return kAck;
  	
  }
	    
//TODO(SRLM): For some reason, this function locks up in the for loop when optimized.
#pragma GCC optimize ("0")
  unsigned char Get(unsigned char device, unsigned char address, char * bytes, int size)
  {
  	if(address == (0x28 | 0x80)) //OUT_X_L with autoincrement bit set
  	{
  		for(volatile int i = 0; i < size; ++i)
  		{
  			bytes[i] = xyz[i];
//			printf("\nbytes[%i] = 0x%X  ", i, bytes[i]);
  		}
  	}
  	else if(address == (0x03 | 0x80)) //OUT_X_L with autoincrement bit set
  	{
  		for(volatile int i = 0; i < size; ++i)
  		{
  			bytes[i] = xyz[i];
//			printf("\nbytes[%i] = 0x%X  ", i, bytes[i]);
  		}
  	}
  	
  	else
  	{
	  	return kNak;
	}

  	return kAck;

  }
  
// -----------------------------------------------------------------------------
  int GetPutStack()
  {
  	if(putStackTop == -1) return -1;
  	else return putStack[putStackTop--];
  }

  int SetXYZ(char * bytes, int size)
  {
  	for(int i = 0; i < size; i++)
  	{
  		xyz[i] = bytes[i];
//		printf("\nxyz[%i] = 0x%X  ", i, xyz[i]);
  	}
  }   
};

#endif // Unit_Test

#endif
