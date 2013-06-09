/* 
 * File:   Sensors.cpp
 * Author: clewis
 * 
 * Created on April 25, 2013, 5:45 PM
 */

#include "Sensors.h"

enum LogLevel {
    kAll, kFatal, kError, kWarn, kInfo, kDebug
};
extern void LogStatusElement(const LogLevel, const char *);

Sensors::Sensors(void) {
    fuel_soc = 0;
    fuel_rate = 0;
    fuel_voltage = kDefaultFuelVoltage;

    automaticRead = false;
    
    controlledIntoBuffer = false;

    lockID = locknew();
    if (lockID < 0) {
        //Something is wrong!
        //TODO(SRLM): fix that something.
    }

    killed = false;

    for (int i = 0; i < SensorTypeLength; i++) {
        readControl[i] = false;
    }

}

Sensors::~Sensors(void) {
    if (lockID >= 0 and lockID <= 7) {
        lockret(lockID);
    }
}

void Sensors::init(void) {
    //I2C
    bus = new i2c();
    //bus->Initialize(kPIN_EEPROM_SCL, kPIN_EEPROM_SDA); //For Beta Boards
    bus->Initialize(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA); //For Beta2 Boards

    fuel = new MAX17048(bus);
    if (fuel->GetStatus() == false) {
        LogStatusElement( kError, "Failed to initialize the MAX17048");
    } else {
        ReadFuel();
    }

    lsm = new LSM303DLHC;
    if (!lsm->Init(bus)) {
        LogStatusElement(kError, "Failed to initialize the LSM303DLHC.");
    }

    l3g = new L3GD20;
    if (!l3g->Init(bus)) {
        LogStatusElement( kError, "Failed to initialize the L3GD20.");
    }

    rtc = new PCF8523(bus, board::kPIN_PCF8523_SQW);
    if (rtc->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the PCF8523.");
    }

    baro = new MS5611(bus);
    if (baro->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the MS5611.");
    }

#ifdef EXTERNAL_IMU
    //Second Bus for additional sensors.
    bus2 = new i2c();
    bus2->Initialize(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2); //For Beta2 Boards

    lsm2 = new LSM303DLHC;
    if (!lsm2->Init(bus2)) {
        LogStatusElement(kError, "Failed to initialize the external LSM303DLHC.");
    }

    l3g2 = new L3GD20;
    if (!l3g2->Init(bus2)) {
        LogStatusElement(kError, "Failed to initialize the external L3GD20.");
    }
#endif

    //GPS
    gps = new MTK3339(board::kPIN_GPS_RX, board::kPIN_GPS_TX,
            board::kPIN_GPS_FIX);
    if (gps->GetStatus() == false) {
        LogStatusElement(kError, "Failed to initialize the GPS.");
    }
}

void Sensors::Server(void) {
    while (!killed) {
        if (automaticRead == true) {
            AutoRead();
        } else {
            
            ControlledRead();
            //waitcnt(CLKFREQ / 100 + CNT); //Sleep for the power savings
        }
    }
}

void Sensors::ControlledRead() {
    char * gpsString = NULL;
    if (readControl[kAccl]) {
        ReadAccl();
        if (controlledIntoBuffer) {
            PIB::_3x2('A', CNT, accl_x, accl_y, accl_z);
        }
        readControl[kAccl] = false;
    }

    if (readControl[kGyro]) {
        ReadGyro();
        if (controlledIntoBuffer) {
            PIB::_3x2('G', CNT, gyro_x, gyro_y, gyro_z);
        }
        readControl[kGyro] = false;
    }

    if (readControl[kMagn]) {
        ReadMagn();
        if (controlledIntoBuffer) {
            PIB::_3x2('M', CNT, magn_x, magn_y, magn_z);
        }
        readControl[kMagn] = false;
    }
    if (readControl[kFuel]) {
        ReadFuel();
        if (controlledIntoBuffer) {
            PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);
        }
        readControl[kFuel] = false;
    }
    if (readControl[kTime]) {
        ReadDateTime();
        if (controlledIntoBuffer) {
            PIB::_3x2('T', CNT, hour, minute, second);
            PIB::_3x2('D', CNT, year, month, day);
        }
        readControl[kTime] = false;
    }

    if (readControl[kBaro] && baro->Touch() == true) {
        ReadBaro();
        if (controlledIntoBuffer) {
            PIB::_2x4('E', CNT, pressure, temperature);
        }
        readControl[kBaro] = false;
    }


    if (readControl[kGPS] && (gpsString = gps->Get()) != NULL) {
        if (controlledIntoBuffer) {
            PIB::_string('P', CNT, gpsString, '\0');
        }
        readControl[kGPS] = false;
    }


#ifdef EXTERNAL_IMU
    if (readControl[kAccl2]) {
        ReadAccl2();
        if (controlledIntoBuffer) {
            PIB::_3x2('B', CNT, accl2_x, accl2_y, accl2_z);
        }
        readControl[kAccl2] = false;
    }

    if (readControl[kGyro2]) {
        ReadGyro2();
        if (controlledIntoBuffer) {
            PIB::_3x2('H', CNT, gyro2_x, gyro2_y, gyro2_z);
        }
        readControl[kGyro2] = false;
    }

    if (readControl[kMagn2]) {
        ReadMagn2();
        if (controlledIntoBuffer) {
            PIB::_3x2('N', CNT, magn2_x, magn2_y, magn2_z);
        }
        readControl[kMagn2] = false;
    }
#endif

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

#ifdef EXTERNAL_IMU	
    Scheduler accl2Scheduler(150 * 10);
    Scheduler gyro2Scheduler(100 * 10);
    Scheduler magn2Scheduler(25 * 10);
#endif


    //Make sure to log fuel at least once...
    ReadFuel();
    PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);

    while (automaticRead == true) {

        //Note: each Put into the buffer must have a matching Get! Otherwise, on
        //occasion the get test will misinterpret a data byte as the start of a
        //packet.

        if (acclScheduler.Run()) {
            ReadAccl();
            PIB::_3x2('A', CNT, accl_x, accl_y, accl_z);
            readControl[kAccl] = false;
        }

        if (gyroScheduler.Run()) {
            ReadGyro();
            PIB::_3x2('G', CNT, gyro_x, gyro_y, gyro_z);
            readControl[kGyro] = false;
        }

        if (magnScheduler.Run()) {
            ReadMagn();
            PIB::_3x2('M', CNT, magn_x, magn_y, magn_z);
            readControl[kMagn] = false;
        }
        if (fuelScheduler.Run()) {
            ReadFuel();
            PIB::_3x2('F', CNT, fuel_voltage, fuel_soc, fuel_rate);
            readControl[kFuel] = false;
        }
        if (timeScheduler.Run()) {
            ReadDateTime();
            PIB::_3x2('T', CNT, hour, minute, second);
            PIB::_3x2('D', CNT, year, month, day);
            readControl[kTime] = false;
        }

        if (baro->Touch() == true) {
            ReadBaro();
            PIB::_2x4('E', CNT, pressure, temperature);
            readControl[kBaro] = false;
        }


        if ((gpsString = gps->Get()) != NULL) {
            PIB::_string('P', CNT, gpsString, '\0');
            readControl[kGPS] = false;
        }


#ifdef EXTERNAL_IMU
        if (accl2Scheduler.Run()) {
            ReadAccl2();
            PIB::_3x2('B', CNT, accl2_x, accl2_y, accl2_z);
            readControl[kAccl2] = false;
        }

        if (gyro2Scheduler.Run()) {
            ReadGyro2();
            PIB::_3x2('H', CNT, gyro2_x, gyro2_y, gyro2_z);
            readControl[kGyro2] = false;
        }

        if (magn2Scheduler.Run()) {
            ReadMagn2();
            PIB::_3x2('N', CNT, magn2_x, magn2_y, magn2_z);
            readControl[kMagn2] = false;
        }
#endif


    }

}

void Sensors::KillServer(void) {
    killed = true;
}




void Sensors::Update(SensorType type, bool new_putIntoBuffer) {
    if (type != kNone) {
        controlledIntoBuffer = new_putIntoBuffer;
        
        readControl[type] = true;
        while (readControl[type]) {

            //Temp to help debug
            extern Serial * debug;
            if(new_putIntoBuffer == true){
                //debug->Put('.');
            }else{
                //debug->Put('*');
            }//*/
        }
    }
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
#ifdef EXTERNAL_IMU
    PIB::_3x4('B' | 0x80, CNT, acclScale, acclScale, acclScale);
#endif

    //Magn
    const float magnScaleFloatXY = 4.347826e-7f; // 1/230gauss * 0.0001 tesla
    const int magnScaleXY = *(int *) &magnScaleFloatXY;
    const float magnScaleFloatZ = 4.87804878e-7f; // 1/205gauss * 0.0001 tesla
    const int magnScaleZ = *(int *) &magnScaleFloatZ;
    PIB::_3x4('M' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);
#ifdef EXTERNAL_IMU
    PIB::_3x4('N' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);
#endif

    const float gyroScaleFloat = 0.001221730475f; // 0.070degrees * 0.0174532925 radians
    const int gyroScale = *(int *) &gyroScaleFloat;
    PIB::_3x4('G' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
#ifdef EXTERNAL_IMU
    PIB::_3x4('H' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
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

void Sensors::SetAutomaticRead(bool new_value) {
    automaticRead = new_value;
}




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