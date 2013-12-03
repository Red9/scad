#include "Sensors.h"


volatile int Sensors::fuel_soc;
volatile int Sensors::fuel_rate;
volatile int Sensors::fuel_voltage;

//Warning: these are not volatile!
int Sensors::year, Sensors::month, Sensors::day, Sensors::hour, Sensors::minute, Sensors::second;

int Sensors::pressure, Sensors::temperature;

int Sensors::gyro_x, Sensors::gyro_y, Sensors::gyro_z;
int Sensors::accl_x, Sensors::accl_y, Sensors::accl_z;
int Sensors::magn_x, Sensors::magn_y, Sensors::magn_z;


#ifdef GAMMA
I2C Sensors::bus1;
I2C Sensors::bus2;
#elif BETA2
I2C Sensors::bus;
#endif


LSM303DLHC Sensors::lsm;
L3GD20 Sensors::l3g;

PCF8523 Sensors::rtc;
MAX17048 Sensors::fuel;
MS5611 Sensors::baro;

MTK3339 Sensors::gps;

volatile bool Sensors::killed;
volatile bool Sensors::logging;

int Sensors::stack[stackSize];

const char * const Sensors::sensorJSON = R"SENSORS(
		"accelerometer":{
			"name":"ACCL",
			"id":"A",
			"processor":{
				"numbers":{
					"axes":["x","y","z"],
					"units":["m/s^2","m/s^2","m/s^2"],
					"bitsPerAxis":16,
					"scalable":true
					
				}
			}
		},
		"gyroscope":{
			"name":"GYRO",
			"id":"G",
			"processor":{
				"numbers":{
					"axes":["x","y","z"],
					"units":["rad/s","rad/s","rad/s"],
					"bitsPerAxis":16,
					"scalable":true
					
				}
			}
		},
		"magnetometer":{
			"name":"MAGN",
			"id":"M",
			"processor":{
				"numbers":{
					"axes":["x","y","z"],
					"units":["tesla","tesla","tesla"],
					"bitsPerAxis":16,
					"scalable":true
					
				}
			}
		},
		"barometer":{
			"name":"BARO",
			"id":"E",
			"processor":{
				"numbers":{
					"axes":["pressure","temperature"],
					"units":["Pa","C"],
					"bitsPerAxis":32,
					"scalable":true
					
				}
			}
		},
		"gps":{
			"name":"GPS",
			"id":"P",
			"processor":{
				"gpsString":{
					"frequency":10
				}
			}
		}
)SENSORS";