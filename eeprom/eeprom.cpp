#include "eeprom.h"

void Eeprom::Start(int scl, int sda){
    base.Initialize(scl, sda);
}


bool Eeprom::PollForAcknowledge(){
	base.Start();
	int counter = 0;
	while(base.SendByte(device) == false){      //device EEPROM write
		if(++counter == 100){ //timeout
			return false;
		}
		base.Stop();
		base.Start();
	}
	return true;
}




bool Eeprom::Put(unsigned short address, char bytes [], int size){
//The lower seven bits define an EEPROM page, so we need a bit of magic to make
//sure that if we go over the boundry, we start a new page.
	
	int bytesWritten = 0;
	while(bytesWritten < size){
		if(PollForAcknowledge() == false){
			return false;
		}
		base.SendByte((address >> 8) & 0xFF); //High address byte
		base.SendByte(address & 0xFF);        //Low address byte
		do{
			base.SendByte(bytes[bytesWritten]);                  //Payload	
			bytesWritten++;
			address++;
		}while((address & 0b1111111) != 0  && bytesWritten < size); //detect rollover
		
		base.Stop();
	}
	
	return true;
}

int Eeprom::Get(unsigned short address, char bytes [], int size){
	int bytesRead = 0;
	while(bytesRead < size){
		if(PollForAcknowledge() == false){
			return -1;
		}
		base.SendByte((address >> 8) & 0xFF); //High address byte
		base.SendByte(address & 0xFF);        //Low address byte
		base.Start();
		base.SendByte(device | 0x01);            //device EEPROM read (w/ read bit set)

		
		while( ((address + 1) & 0b1111111) != 0
		      && bytesRead + 1 < size)
		{
			bytes[bytesRead] = base.ReadByte(true);
			bytesRead++;
			address++;
		}
		
		bytes[bytesRead] = base.ReadByte(false);
		bytesRead++;
		address++;		
				
		base.Stop();
	}
	
	return 0;

}
bool Eeprom::Put(unsigned short address, int bytes, int length){
	
	char temp[4];
	//Even if length is < 4, do them all (easier than a loop). Only the used ones will be written.
	temp[3] = (((unsigned)bytes) & 0xFF000000) >> 24;
	temp[2] = (((unsigned)bytes) & 0xFF0000  ) >> 16;
	temp[1] = (((unsigned)bytes) & 0xFF00    ) >>  8;
	temp[0] = (((unsigned)bytes) & 0xFF      ) >>  0;
	return Put(address, temp, length);
}

int Eeprom::Get(unsigned short address, int length){
	char temp[4];
	Get(address, temp, length);
	int result = 0;
	for(int i = length-1; i >= 0; --i){
		result = (result << 8) | temp[i];
	}
	return result;
}



bool Eeprom::Put(unsigned short address, char byte)
{	
	char bytes[] = {byte};
	return Put(address, bytes, 1);
}

int Eeprom::Get(unsigned short address)
{
	char byte[1];
	int result = Get(address, byte,1);
	if(result < 0){
		return result;
	}else{
		return byte[0];
	}
}
