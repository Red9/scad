/* 
 * File:   Sensors.cpp
 * Author: clewis
 * 
 * Created on April 25, 2013, 5:45 PM
 */

#include "Sensors.h"

extern volatile bool datalogging;

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
extern void LogStatusElement(ConcurrentBuffer *, const LogLevel, const char *);

Sensors::Sensors() {
    fuel_soc = 0;
    fuel_rate = 0;
    fuel_voltage = kDefaultFuelVoltage;

    lockID = locknew();
    if (lockID < 0) {
        //Something is wrong!
        //TODO(SRLM): fix that something.
    }

    ConcurrentBuffer * buffer = new ConcurrentBuffer();

    killed = false;

    for (int i = 0; i < SensorTypeLength; i++) {
        readControl[i] = false;
    }

}

Sensors::~Sensors() {
    if (lockID >= 0 and lockID <= 7) {
        lockret(lockID);
    }

    delete buffer;
    buffer = NULL;
}

void Sensors::init() {
    ConcurrentBuffer * buffer = new ConcurrentBuffer();

    //I2C
    bus = new i2c();
    //bus->Initialize(kPIN_EEPROM_SCL, kPIN_EEPROM_SDA); //For Beta Boards
    bus->Initialize(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA); //For Beta2 Boards

    fuel = new MAX17048(bus);
    if (fuel->GetStatus() == false) {
        LogStatusElement(buffer, kError, "Failed to initialize the MAX17048");
    } else {
        ReadFuel();
    }

    lsm = new LSM303DLHC;
    if (!lsm->Init(bus)) {
        LogStatusElement(buffer, kError, "Failed to initialize the LSM303DLHC.");
    }

    l3g = new L3GD20;
    if (!l3g->Init(bus)) {
        LogStatusElement(buffer, kError, "Failed to initialize the L3GD20.");
    }

    rtc = new PCF8523(bus, board::kPIN_PCF8523_SQW);
    if (rtc->GetStatus() == false) {
        LogStatusElement(buffer, kError, "Failed to initialize the PCF8523.");
    }

    baro = new MS5611(bus);
    if (baro->GetStatus() == false) {
        LogStatusElement(buffer, kError, "Failed to initialize the MS5611.");
    }

#ifdef EXTERNAL_IMU
    //Second Bus for additional sensors.
    bus2 = new i2c();
    bus2->Initialize(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2); //For Beta2 Boards

    lsm2 = new LSM303DLHC;
    if (!lsm2->Init(bus2)) {
        LogStatusElement(buffer, kError, "Failed to initialize the external LSM303DLHC.");
    }

    l3g2 = new L3GD20;
    if (!l3g2->Init(bus2)) {
        LogStatusElement(buffer, kError, "Failed to initialize the external L3GD20.");
    }
#endif

    //GPS
    gps = new MTK3339(board::kPIN_GPS_RX, board::kPIN_GPS_TX,
            board::kPIN_GPS_FIX);
    if (gps->GetStatus() == false) {
        LogStatusElement(buffer, kError, "Failed to initialize the GPS.");
    }
}





void Sensors::Server(void) {
    while (!killed) {
        if (datalogging) {
            AutoRead();
        } else {
            ControlledRead();
            waitcnt(CLKFREQ / 100 + CNT); //Sleep for the power savings
        }
    }
}

void Sensors::ControlledRead(void) {
    char * gpsString = NULL;
    if (readControl[kAccl]) {
        ReadAccl();
        PIB::_3x2(buffer, 'A', CNT, accl_x, accl_y, accl_z);
        readControl[kAccl] = false;
    }

    if (readControl[kGyro]) {
        ReadGyro();
        PIB::_3x2(buffer, 'G', CNT, gyro_x, gyro_y, gyro_z);
        readControl[kGyro] = false;
    }

    if (readControl[kMagn]) {
        ReadMagn();
        PIB::_3x2(buffer, 'M', CNT, magn_x, magn_y, magn_z);
        readControl[kMagn] = false;
    }
    if (readControl[kFuel]) {
        ReadFuel();
        PIB::_3x2(buffer, 'F', CNT, fuel_voltage, fuel_soc, fuel_rate);
        readControl[kMagn] = false;
    }
    if (readControl[kTime]) {
        ReadDateTime();
        PIB::_3x2(buffer, 'T', CNT, hour, minute, second);
        PIB::_3x2(buffer, 'D', CNT, year, month, day);
        readControl[kTime] = false;
    }

    if (readControl[kBaro] && baro->Touch() == true) {
        ReadBaro();
        PIB::_2x4(buffer, 'E', CNT, pressure, temperature);
        readControl[kBaro] = false;
    }


    if (readControl[kGPS] && (gpsString = gps->Get()) != NULL) {
        PIB::_string(buffer, 'P', CNT, gpsString, '\0');
        readControl[kGPS] = false;
    }


#ifdef EXTERNAL_IMU
    if (readControl[kAccl2]) {
        ReadAccl2();
        PIB::_3x2(buffer, 'B', CNT, accl2_x, accl2_y, accl2_z);
        readControl[kAccl2] = false;
    }

    if (readControl[kGyro2]) {
        ReadGyro2();
        PIB::_3x2(buffer, 'H', CNT, gyro2_x, gyro2_y, gyro2_z);
        readControl[kGyro2] = false;
    }

    if (readControl[kMagn2]) {
        ReadMagn2();
        PIB::_3x2(buffer, 'N', CNT, magn2_x, magn2_y, magn2_z);
        readControl[kMagn2] = false;
    }
#endif

}

void Sensors::AutoRead(void) {


    AddScales(buffer);

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

#ifdef EXTERNAL_IMU	
    Scheduler accl2Scheduler(150 * 10);
    Scheduler gyro2Scheduler(100 * 10);
    Scheduler magn2Scheduler(25 * 10);
#endif


    //Make sure to log fuel at least once...
    ReadFuel();
    PIB::_3x2(buffer, 'F', CNT, fuel_voltage, fuel_soc, fuel_rate);

    while (datalogging) {

        //Note: each Put into the buffer must have a matching Get! Otherwise, on
        //occasion the get test will misinterpret a data byte as the start of a
        //packet.

        if (acclScheduler.Run()) {
            ReadAccl();
            PIB::_3x2(buffer, 'A', CNT, accl_x, accl_y, accl_z);
            readControl[kAccl] = false;
        }

        if (gyroScheduler.Run()) {
            ReadGyro();
            PIB::_3x2(buffer, 'G', CNT, gyro_x, gyro_y, gyro_z);
            readControl[kGyro] = false;
        }

        if (magnScheduler.Run()) {
            ReadMagn();
            PIB::_3x2(buffer, 'M', CNT, magn_x, magn_y, magn_z);
            readControl[kMagn] = false;
        }
        if (fuelScheduler.Run()) {
            ReadFuel();
            PIB::_3x2(buffer, 'F', CNT, fuel_voltage, fuel_soc, fuel_rate);
            readControl[kFuel] = false;
        }
        if (timeScheduler.Run()) {
            ReadDateTime();
            PIB::_3x2(buffer, 'T', CNT, hour, minute, second);
            PIB::_3x2(buffer, 'D', CNT, year, month, day);
            readControl[kTime] = false;
        }

        if (baro->Touch() == true) {
            ReadBaro();
            PIB::_2x4(buffer, 'E', CNT, pressure, temperature);
            readControl[kBaro] = false;
        }


        if ((gpsString = gps->Get()) != NULL) {
            PIB::_string(buffer, 'P', CNT, gpsString, '\0');
            readControl[kGPS] = false;
        }


#ifdef EXTERNAL_IMU
        if (accl2Scheduler.Run()) {
            ReadAccl2();
            PIB::_3x2(buffer, 'B', CNT, accl2_x, accl2_y, accl2_z);
            readControl[kAccl2] = false;
        }

        if (gyro2Scheduler.Run()) {
            ReadGyro2();
            PIB::_3x2(buffer, 'H', CNT, gyro2_x, gyro2_y, gyro2_z);
            readControl[kGyro2] = false;
        }

        if (magn2Scheduler.Run()) {
            ReadMagn2();
            PIB::_3x2(buffer, 'N', CNT, magn2_x, magn2_y, magn2_z);
            readControl[kMagn2] = false;
        }
#endif


    }

}

void Sensors::KillServer(void) {
    killed = true;
}

void Sensors::Update(SensorType type) {
    readControl[type] = true;
    while (readControl[type]) {
    }
}

void Sensors::AddScales(ConcurrentBuffer * buffer) {
    //Baro

    const float baroScaleFloatPressure = 1.0f; // 0.01millibar * 100pascals
    const float baroScaleFloatTemperature = 0.01f;
    const int baroScalePressure = *(int *) &baroScaleFloatPressure;
    const int baroScaleTemperature = *(int *) &baroScaleFloatTemperature;
    PIB::_2x4(buffer, 'E' | 0x80, CNT, baroScalePressure, baroScaleTemperature);

    //Accl
    const float acclScaleFloat = 0.00735f; // 0.012g / 16 * 9.8m/s^2
    const int acclScale = *(int *) &acclScaleFloat;
    PIB::_3x4(buffer, 'A' | 0x80, CNT, acclScale, acclScale, acclScale);
#ifdef EXTERNAL_IMU
    PIB::_3x4(buffer, 'B' | 0x80, CNT, acclScale, acclScale, acclScale);
#endif

    //Magn
    const float magnScaleFloatXY = 4.347826e-7f; // 1/230gauss * 0.0001 tesla
    const int magnScaleXY = *(int *) &magnScaleFloatXY;
    const float magnScaleFloatZ = 4.87804878e-7f; // 1/205gauss * 0.0001 tesla
    const int magnScaleZ = *(int *) &magnScaleFloatZ;
    PIB::_3x4(buffer, 'M' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);
#ifdef EXTERNAL_IMU
    PIB::_3x4(buffer, 'N' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);
#endif

    const float gyroScaleFloat = 0.001221730475f; // 0.070degrees * 0.0174532925 radians
    const int gyroScale = *(int *) &gyroScaleFloat;
    PIB::_3x4(buffer, 'G' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
#ifdef EXTERNAL_IMU
    PIB::_3x4(buffer, 'H' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
#endif

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

#ifdef EXTERNAL_IMU

void Sensors::ReadGyro2(void) {
    l3g2->ReadGyro(gyro2_x, gyro2_y, gyro2_z);
}

void Sensors::ReadAccl2(void) {
    lsm2->ReadAccl(accl2_x, acc2_y, accl2_z);
}

void Sensors::ReadMagn2(void) {
    lsm2->ReadMagn(magn2_x, magn2_y, magn2_z);
}
#endif





/*
bool Sensors::GetLock(int timeout) {
    //Check once at the beginning to improve performance.
    if (lockset(lockID) == false) {
        return true;
    }

    if (timeout < 0) {
        //Loop while the lock is out (previous state == true)
        while (lockset(lockID) != false) {
        }
        return true;
    } else {
        int tout = (CLKFREQ / 1000) * timeout;
        int totaltime = 0;
        int previous_cnt = CNT;
        int current_cnt;
        do {
            if (lockset(lockID) == false) {
                return true;
            }
            current_cnt = CNT;
            totaltime += current_cnt - previous_cnt;
            previous_cnt = current_cnt;

        } while (totaltime < tout);
        return false;
    }
}

void Sensors::ReturnLock(void) {
    lockclr(lockID);
}
 */