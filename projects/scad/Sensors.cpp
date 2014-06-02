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
libpropeller::I2C Sensors::bus1;
libpropeller::I2C Sensors::bus2;
#elif BETA2
libpropeller::I2C Sensors::bus;
#endif


libpropeller::LSM303DLHC Sensors::lsm;
libpropeller::L3GD20 Sensors::l3g;

libpropeller::PCF8523 Sensors::rtc;
libpropeller::MAX17048 Sensors::fuel;
libpropeller::MS5611 Sensors::baro;

libpropeller::MTK3339 Sensors::gps;

volatile bool Sensors::killed;
volatile bool Sensors::logging;

int Sensors::stack[stackSize];


volatile bool Sensors::set_clock;
volatile int Sensors::set_year;
volatile int Sensors::set_month;
volatile int Sensors::set_day;
volatile int Sensors::set_hour;
volatile int Sensors::set_minute;
volatile int Sensors::set_second;


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