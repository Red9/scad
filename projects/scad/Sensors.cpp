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
i2c Sensors::bus1;
i2c Sensors::bus2;
#elif BETA2
i2c Sensors::bus;
#endif


LSM303DLHC * Sensors::lsm;
L3GD20 * Sensors::l3g;

PCF8523 * Sensors::rtc;
MAX17048 * Sensors::fuel;
MTK3339 * Sensors::gps;
MS5611 * Sensors::baro;

volatile bool Sensors::paused;

volatile bool Sensors::killed;

int Sensors::stack[stackSize];

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
extern void LogStatusElement(const LogLevel, const char *);

void Sensors::init(void) {

    fuel_soc = 0;
    fuel_rate = 0;
    fuel_voltage = kDefaultFuelVoltage;

    paused = false;
    killed = false;

    //I2C
    //bus = new i2c();
#ifdef GAMMA
    bus1.Initialize(board::kPIN_I2C_SCL_1, board::kPIN_I2C_SDA_1);
    bus2.Initialize(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2);
    i2c * bus1addr = &bus1;
    i2c * bus2addr = &bus2;
#elif BETA2
    bus.Initialize(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA); //For Beta2 Boards
    i2c * bus1addr = &bus;
    i2c * bus2addr = &bus;
#endif

    LogStatusElement(kInfo, "I2C Bus Initialized.");


    fuel = new MAX17048(bus2addr);
    if (fuel->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the MAX17048");
    } else {
        ReadFuel();
        LogStatusElement(kInfo, "Fuel gauge initialized");
    }

    lsm = new LSM303DLHC;
    if (!lsm->Init(bus1addr)) {
        LogStatusElement(kError, "Failed to initialize the LSM303DLHC.");
    } else {
        LogStatusElement(kInfo, "Accelerometer and Magnetometer initialized");
    }

    l3g = new L3GD20;
    if (!l3g->Init(bus1addr)) {
        LogStatusElement(kError, "Failed to initialize the L3GD20.");
    } else {
        LogStatusElement(kInfo, "Gyro initialized");
    }

    rtc = new PCF8523(bus2addr);
    if (rtc->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the PCF8523.");
    } else {
        LogStatusElement(kInfo, "RTC initialized");
    }

    baro = new MS5611(bus2addr);
    if (baro->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the MS5611.");
    } else {
        LogStatusElement(kInfo, "Barometer initialized");
    }

    LogStatusElement(kInfo, "Preparing GPS");

    //GPS
    gps = new MTK3339(board::kPIN_GPS_RX, board::kPIN_GPS_TX);
    if (gps->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the GPS.");
    }

    LogStatusElement(kInfo, "GPS Initialized.");
}

void Sensors::Start(void) {
    init();
    cogstart(Server, NULL, stack, stackSize);
}

void Sensors::Server(void * temp) {
    while (!killed) {
        AutoRead();
    }
    killed = false;
    cogstop(cogid());
}

void Sensors::AutoRead(void) {


    AddScales();

    char * gpsString = NULL;
    //Flush GPS buffer.
    while ((gpsString = gps->Get()) != NULL) {
        /*Throw away stings*/
    }

    Scheduler acclScheduler(150 * 10);
    Scheduler gyroScheduler(100 * 10);
    Scheduler magnScheduler(25 * 10);
    Scheduler fuelScheduler(1); //10 second cycle
    Scheduler timeScheduler(1); //10 second cycle


    //Make sure to log fuel at least once...
    ReadFuel();
    PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);

    while (paused == false) {

        //Note: each Put into the buffer must have a matching Get! Otherwise, on
        //occasion the get test will misinterpret a data byte as the start of a
        //packet.

        if (acclScheduler.Run()) {
            ReadAccl();
            PIB::_3x2('A', CNT, accl_x, accl_y, accl_z);
        }

        if (gyroScheduler.Run()) {
            ReadGyro();
            PIB::_3x2('G', CNT, gyro_x, gyro_y, gyro_z);
        }

        if (magnScheduler.Run()) {
            ReadMagn();
            PIB::_3x2('M', CNT, magn_x, magn_y, magn_z);
        }
        if (fuelScheduler.Run()) {
            ReadFuel();
            PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);
        }
        if (timeScheduler.Run()) {
            ReadDateTime();
            PIB::_3x2('T', CNT, hour, minute, second);
            PIB::_3x2('D', CNT, year, month, day);
        }

        if (baro->Touch() == true) {
            ReadBaro();
            PIB::_2x4('E', CNT, pressure, temperature);
        }


        if ((gpsString = gps->Get()) != NULL) {
            PIB::_string('P', CNT, gpsString, '\0');
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
    const float magnScaleFloatXY = 4.347826e-7f; // 1/230gauss * 0.0001 tesla
    const int magnScaleXY = *(int *) &magnScaleFloatXY;
    const float magnScaleFloatZ = 4.87804878e-7f; // 1/205gauss * 0.0001 tesla
    const int magnScaleZ = *(int *) &magnScaleFloatZ;
    PIB::_3x4('M' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);

    const float gyroScaleFloat = 0.001221730475f; // 0.070degrees * 0.0174532925 radians
    const int gyroScale = *(int *) &gyroScaleFloat;
    PIB::_3x4('G' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
}

void Sensors::ReadDateTime(void) {
    rtc->GetClock(year, month, day, hour, minute, second);
}

void Sensors::ReadGyro(void) {
    l3g->ReadGyro(gyro_x, gyro_y, gyro_z);
}

void Sensors::ReadAccl(void) {
    lsm->ReadAccl(accl_x, accl_y, accl_z);
}

void Sensors::ReadMagn(void) {
    lsm->ReadMagn(magn_x, magn_y, magn_z);
}

void Sensors::ReadFuel() {
    fuel_soc = fuel->GetStateOfCharge();
    fuel_voltage = fuel->GetVoltage();
    fuel_rate = fuel->GetChargeRate();
}

void Sensors::ReadBaro() {
    baro->Get(pressure, temperature, true);
}

void Sensors::PauseReading(void) {
    paused = true;
}

void Sensors::ResumeReading(void) {
    paused = false;
}
