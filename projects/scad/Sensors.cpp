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

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
extern void LogStatusElement(const LogLevel, const char *);

void Sensors::init(void) {

    fuel_soc = 0;
    fuel_rate = 0;
    fuel_voltage = kDefaultFuelVoltage;

    killed = false;
    logging = false;

#ifdef GAMMA
    bus1.Init(board::kPIN_I2C_SCL_1, board::kPIN_I2C_SDA_1, 444000);
    bus2.Init(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2, 444000);
    I2C * bus1addr = &bus1;
    I2C * bus2addr = &bus2;
#elif BETA2
    bus.Init(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA, 444000); //For Beta2 Boards
    I2C * bus1addr = &bus;
    I2C * bus2addr = &bus;
#endif

    LogStatusElement(kInfo, "I2C Bus Initialized.");

    if (fuel.Init(bus2addr) == false) {
        LogStatusElement(kError, "Failed to initialize the MAX17048");
    } else {
        ReadFuel();
        LogStatusElement(kInfo, "Fuel gauge initialized");
    }

    if (lsm.Init(bus1addr) == false) {
        LogStatusElement(kError, "Failed to initialize the LSM303DLHC.");
    } else {
        LogStatusElement(kInfo, "Accelerometer and Magnetometer initialized");
    }

    if (l3g.Init(bus1addr, L3GD20::LSB_1) == false) {
        LogStatusElement(kError, "Failed to initialize the L3GD20.");
    } else {
        LogStatusElement(kInfo, "Gyro initialized");
    }

    if (rtc.Init(bus2addr) == false) {
        LogStatusElement(kError, "Failed to initialize the PCF8523.");
    } else {
        LogStatusElement(kInfo, "RTC initialized");
    }


    if (baro.Init(bus2addr, MS5611::LSB_1) == false) {
        LogStatusElement(kError, "Failed to initialize the MS5611.");
    } else {
        LogStatusElement(kInfo, "Barometer initialized");
    }

    LogStatusElement(kInfo, "Preparing GPS");

    //GPS
    gps.Start(board::kPIN_GPS_RX, board::kPIN_GPS_TX);
    if (gps.GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the GPS.");
    }

    LogStatusElement(kInfo, "GPS Initialized.");
}

void Sensors::Start(void) {
    init();
    cogstart(Server, NULL, stack, stackSize);
}

void Sensors::Server(void * temp) {
    AutoRead();
    killed = false; // Set flag to indicate done.
    cogstop(cogid());
}

void Sensors::AutoRead(void) {

    char * gpsString = NULL;
    //Flush GPS buffer.
    while ((gpsString = gps.Get()) != NULL) {
        /*Throw away stings*/
    }

    Scheduler acclScheduler(150 * 10);
    Scheduler gyroScheduler(100 * 10);
    Scheduler magnScheduler(25 * 10);
    Scheduler fuelScheduler(1); //10 second cycle
    Scheduler timeScheduler(1); //10 second cycle


    //Make sure to read fuel and time at least once...
    ReadFuel();
    ReadDateTime();


    while (killed == false) {

        if (acclScheduler.Run()) {
            ReadAccl();
            if (logging == true) {
                PIB::_3x2('A', CNT, accl_x, accl_y, accl_z);
            }
        }

        if (gyroScheduler.Run()) {
            ReadGyro();
            if (logging == true) {
                PIB::_3x2('G', CNT, gyro_x, gyro_y, gyro_z);
            }
        }

        if (magnScheduler.Run()) {
            ReadMagn();
            if (logging == true) {
                PIB::_3x2('M', CNT, magn_x, magn_y, magn_z);
            }
        }
        if (fuelScheduler.Run()) {
            ReadFuel();
            if (logging == true) {
                PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);
            }
        }
        if (timeScheduler.Run()) {
            ReadDateTime();
            if (logging == true) {
                PIB::_3x2('T', CNT, hour, minute, second);
                PIB::_3x2('D', CNT, year, month, day);
            }
        }

        if (baro.Touch() == true) {
            ReadBaro();
            if (logging == true) {
                PIB::_2x4('E', CNT, pressure, temperature);
            }
        }


        if ((gpsString = gps.Get()) != NULL) {
            if (logging == true) {
                PIB::_string('P', CNT, gpsString, '\0');
            }
        }
    }

}

void Sensors::Stop(void) {
    killed = true;
    while (killed == true) {
    } //Spin until the cog indicates that it's done.
}

void Sensors::AddScales() {
    //Baro
    const float baroScaleFloatPressure = 1.0f; // 0.01millibar * 100pascals
    const float baroScaleFloatTemperature = 0.01f;
    const int baroScalePressure = *(int *) &baroScaleFloatPressure;
    const int baroScaleTemperature = *(int *) &baroScaleFloatTemperature;
    PIB::_2x4('E' | 0x80, CNT, baroScalePressure, baroScaleTemperature);

    //Accl
    const float acclScaleFloat = 0.00735f; // 0.012g / 16 * 9.8m/s^2
    const int acclScale = *(int *) &acclScaleFloat;
    PIB::_3x4('A' | 0x80, CNT, acclScale, acclScale, acclScale);


    //Magn
    //const float magnScaleFloatXY = 4.347826e-7f; // 1/230gauss * 0.0001 tesla
    //const float magnScaleFloatXY = 1.16959e-7f; // 1/855gauss * 0.0001 tesla
    const float magnScaleFloatXY = 9.09091e-8f; // 1/1100gauss * 0.0001 tesla
    const int magnScaleXY = *(int *) &magnScaleFloatXY;
    //const float magnScaleFloatZ = 4.87804878e-7f; // 1/205gauss * 0.0001 tesla
    //const float magnScaleFloatZ = 1.31579e-7f; // 1/760gauss * 0.0001 tesla
    const float magnScaleFloatZ = 1.02041e-7f; // 1/980gauss * 0.0001 tesla
    const int magnScaleZ = *(int *) &magnScaleFloatZ;
    PIB::_3x4('M' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);

    const float gyroScaleFloat = 0.001221730475f; // 0.070degrees * 0.0174532925 radians
    const int gyroScale = *(int *) &gyroScaleFloat;
    PIB::_3x4('G' | 0x80, CNT, gyroScale, gyroScale, gyroScale);


    //F: 6267046, 4198.000000, 98.000000, 76.000000
    const float fuelScaleFloatVoltage = 0.001;
    const int fuelScaleVoltage = *(int *) &fuelScaleFloatVoltage;
    const float fuelScaleFloatSOC = 1;
    const int fuelScaleSOC = *(int *) &fuelScaleFloatSOC;
    const float fuelScaleFloatRate = 0.1;
    const int fuelScaleRate = *(int *) &fuelScaleFloatRate;
    PIB::_3x4('F' | 0x80, CNT, fuelScaleVoltage, fuelScaleSOC, fuelScaleRate);


}

void Sensors::ReadDateTime(void) {
    rtc.GetClock(year, month, day, hour, minute, second);
}

void Sensors::ReadGyro(void) {
    l3g.ReadGyro(gyro_x, gyro_y, gyro_z);
}

void Sensors::ReadAccl(void) {
    lsm.ReadAccl(accl_x, accl_y, accl_z);
}

void Sensors::ReadMagn(void) {
    lsm.ReadMagn(magn_x, magn_y, magn_z);
}

void Sensors::ReadFuel() {
    fuel_soc = fuel.GetStateOfCharge();
    fuel_voltage = fuel.GetVoltage();
    fuel_rate = fuel.GetChargeRate();
}

void Sensors::ReadBaro() {
    baro.Get(pressure, temperature, true);
}

void Sensors::SetLogging(const bool new_logging) {
    logging = new_logging;
}