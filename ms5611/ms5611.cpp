#include "ms5611.h"


const char MS5611::kPROMRead [] = {	0b10100000, // 16 bit reserved for manufacturer
									0b10100010, // C1
									0b10100100, // C2
									0b10100110, // C3
									0b10101000, // C4
									0b10101010, // C5
									0b10101100, // C6
									0b10101110};// CRC

MS5611::MS5611(i2c * newbus){
	bus = newbus;
	
	GetStatus();
	if(!status){// or bus == NULL){ //TODO(SRLM): is this right?
		return;
	}
	
	D1 = 0; // Pressure
	D2 = 0; // Temperature
	
	//Read PROM here
	int C[6];
	for(int i = 0; i < 6; i++){
	    char data[2];
	    bus->Put(deviceBaro, kPROMRead[i+1]);
	    bus->Get(deviceBaro, data, 2);
	    C[i] = data[0] << 8 | data[1];
	
	}
	SetC(C[0], C[1], C[2], C[3], C[4], C[5]);
	
	convertingTemperature = true;
	bus->Put(deviceBaro, kConvertD2OSR4096);

	newData = false;
	
	conversionValidCNT = CNT + (CLKFREQ*9)/1000;
	
	
}

int MS5611::ExpandReading(const char data[]){
	//MS5611 returns a 24 bit unsigned number.
	return data[0] << 16 | data[1] << 8 | data[2];
}

bool MS5611::Touch(){

	if(CNT < conversionValidCNT //Not ready, simple case
	   || (CNT > 0x7fffFfff && conversionValidCNT < 0x7fffFfff)
	){ 
		return false;
	}

	//Read ADC on MS5611, and get whatever it was converting.
	char data[3];
	
	bus->Put(deviceBaro, kADCRead);
	
	bus->Get(deviceBaro, data, 3);	
	int reading = ExpandReading(data);
	newData = true;
	
	conversionValidCNT = CNT + (CLKFREQ*9)/1000;
	

	if(convertingTemperature){
		D2 = reading;
		//Set ADC to convert pressure
		bus->Put(deviceBaro, kConvertD1OSR4096);
		
		convertingTemperature = false;
		return false;
	}else{
		D1 = reading;
		//Set ADC to convert temperature
		bus->Put(deviceBaro, kConvertD2OSR4096);
		
		convertingTemperature = true;
		return true;
	}			
}

void MS5611::Get(int & tPressure, int & tTemperature,
                 const bool calibrationCalculation){

	if(calibrationCalculation == true){
		if(newData){
			Calculate();
		}
		tPressure    = pressure;
		tTemperature = temperature;
		newData = false;
	}else{
		tPressure    = D1;
		tTemperature = D2;
	}
}

void MS5611::Calculate(void){
//These equations are straight from the MS5611 datasheet.
	int dT = D2 - C5;
	temperature = 2000 + ((dT * C6) >> 23);
	
	int64_t T2 = 0;
	int64_t OFF2 = 0;
	int64_t SENS2 = 0;
	
	if(temperature < 2000){
	
		int64_t dT64 = dT;
		
		T2 = (dT64*dT64) >> 31;
		OFF2 = (5 * (temperature-2000) * (temperature-2000)) >> 1;
		SENS2 = OFF2 >> 1;
		
		if(temperature < -1500){   //Very low temperature
			OFF2 = OFF2 + (7 * (temperature + 1500) * (temperature + 1500));
			SENS2 = SENS2 + ((11 * (temperature + 1500) * (temperature + 1500)) >> 1);
		}
	}
	

	int64_t OFF = C2 + ((C4*dT) >> 7);
	int64_t SENS =C1 + ((C3*dT) >> 8);
	
	temperature = temperature - T2;
	OFF = OFF - OFF2;
	SENS = SENS - SENS2;
	
	pressure = (int)((((((int64_t)D1)*SENS) >> 21) - OFF) >> 15);
}

void MS5611::SetC(const int newC1, const int newC2, const int newC3,
                  const int newC4, const int newC5, const int newC6){
	C1 = ((int64_t)newC1) << 15;
	C2 = ((int64_t)newC2) << 16;
	C3 = (int64_t) newC3;
	C4 = (int64_t) newC4;

	C5 = newC5 << 8;
	C6 = newC6;
}

void MS5611::GetC(int & oldC1, int & oldC2, int & oldC3,
                  int & oldC4, int & oldC5, int & oldC6){
	oldC1 = (int)(C1 >> 15);
	oldC2 = (int)(C2 >> 16);
	oldC3 = (int)C3;
	oldC4 = (int)C4;
	oldC5 = C5 >> 8;
	oldC6 = C6;
}

void MS5611::TEST_SetD(const int newD1, const int newD2){
	D1 = newD1;
	D2 = newD2;
	newData = true;
}

bool MS5611::GetStatus(){
	
	if(bus->Ping(deviceBaro) == bus->kNak){
		status = false;
		return false;
	}else{
		status = true;
		return true;
	}
}

bool MS5611::Reset(){
    return bus->Put(deviceBaro, kReset) == bus->kAck;

}


