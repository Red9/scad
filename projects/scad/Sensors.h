#ifndef PROPGCC_SRLM_SENSORS_H
#define	PROPGCC_SRLM_SENSORS_H

#include <propeller.h>

#include "librednine/concurrent_buffer/concurrent_buffer.h"
#include "librednine/concurrent_buffer/pib.h"
#include "libpropeller/scheduler/scheduler.h"
#include "libpropeller/i2c/i2c.h"
#include "libpropeller/lsm303dlhc/lsm303dlhc.h"
#include "libpropeller/l3gd20/l3gd20.h"
#include "libpropeller/pcf8523/pcf8523.h"
#include "libpropeller/max17048/max17048.h"
#include "libpropeller/ms5611/ms5611.h"
#include "libpropeller/mtk3339/mtk3339.h"

/* Pin definitions */
#ifdef GAMMA
#include "scadgamma.h"
#elif BETA2
#include "scadbeta2.h"
#endif

#ifdef DEBUG_PORT
extern Serial debug;
#endif

class Sensors {
public:
    
    static const char * const sensorJSON;
    static const int kDefaultFuelVoltage = 0;

    static volatile int fuel_soc;
    static volatile int fuel_rate;
    static volatile int fuel_voltage;

    //Warning: these are not volatile!
    static int year, month, day, hour, minute, second;

    static int pressure, temperature;

    static int gyro_x, gyro_y, gyro_z;
    static int accl_x, accl_y, accl_z;
    static int magn_x, magn_y, magn_z;

    static bool Start(void) {
        set_clock = false;
        
        // TODO(SRLM) Add in some intelligence here that checks the return value of init (for error))
        bool result = init();
        cogstart(Server, NULL, stack, stackSize);
        return result;
    }

    /** Kill the server if running.
     * 
     */
    static void Stop(void) {
        killed = true;
        while (killed == true) {
        } //Spin until the cog indicates that it's done.
    }

    static void SetLogging(const bool new_logging) {
        logging = new_logging;
    }

    /**
     * @TODO: Make sure this thread safe!
     */
    static void AddScales(void) {
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
        const float magnScaleFloatXY = 9.09091e-8f; // 1/1100gauss * 0.0001 tesla
        const int magnScaleXY = *(int *) &magnScaleFloatXY;
        const float magnScaleFloatZ = 1.02041e-7f; // 1/980gauss * 0.0001 tesla
        const int magnScaleZ = *(int *) &magnScaleFloatZ;
        PIB::_3x4('M' | 0x80, CNT, magnScaleXY, magnScaleXY, magnScaleZ);

        const float gyroScaleFloat = 0.001221730475f; // 0.070degrees * 0.0174532925 radians
        const int gyroScale = *(int *) &gyroScaleFloat;
        PIB::_3x4('G' | 0x80, CNT, gyroScale, gyroScale, gyroScale);
    }
    
    static const char * GetJSON(){
        return sensorJSON;
    }

    
    static bool SetClock(const int year, const int month, const int day,
    const int hour, const int minute, const int second){
        set_year = year - 2000;
        set_month = month;
        set_day = day;
        set_hour = hour;
        set_minute = minute;
        set_second = second;
        set_clock = true;
        return true;
    }
    
private:
    
    static volatile bool set_clock;
    static volatile int set_year;
    static volatile int set_month;
    static volatile int set_day;
    static volatile int set_hour;
    static volatile int set_minute;
    static volatile int set_second;
    

    /* Pin definitions */
#ifdef GAMMA
    static libpropeller::I2C bus1;
    static libpropeller::I2C bus2;
#elif BETA2
    static libpropeller::I2C bus;
#endif



    static libpropeller::LSM303DLHC lsm;
    static libpropeller::L3GD20 l3g;

    static libpropeller::PCF8523 rtc;
    static libpropeller::MAX17048 fuel;
    static libpropeller::MS5611 baro;

    static libpropeller::MTK3339 gps;

    static volatile bool paused;
    static volatile bool logging;

    static volatile bool killed;





    static const int stackSize = 176 + 200;
    static int stack[stackSize];

    /** Setup to start the sensors Server. Must be called once (and only
     *  once).
     *
     * @return true if successful, false otherwise.
     */
    static bool init(void) {
        bool result = true;

        fuel_soc = 0;
        fuel_rate = 0;
        fuel_voltage = kDefaultFuelVoltage;

        killed = false;
        logging = false;

#ifdef GAMMA
        bus1.Init(board::kPIN_I2C_SCL_1, board::kPIN_I2C_SDA_1, 444000);
        bus2.Init(board::kPIN_I2C_SCL_2, board::kPIN_I2C_SDA_2, 444000);
        libpropeller::I2C * bus1addr = &bus1;
        libpropeller::I2C * bus2addr = &bus2;
#elif BETA2
        bus.Init(board::kPIN_I2C_SCL, board::kPIN_I2C_SDA, 444000); //For Beta2 Boards
        libpropeller::I2C * bus1addr = &bus;
        libpropeller::I2C * bus2addr = &bus;
#endif

        if (fuel.Init(bus2addr) == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init fuel.");
#endif
        } else {
            ReadFuel();
        }

        if (lsm.Init(bus1addr) == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init lsm.");
#endif
        }

        if (l3g.Init(bus1addr, libpropeller::L3GD20::LSB_1) == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init l3gd20.");
#endif
        }

        if (rtc.Init(bus2addr) == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init rtc.");
#endif
        }

        if (baro.Init(bus2addr, libpropeller::MS5611::LSB_1) == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init baro.");
#endif
        }

        //GPS
        gps.Start(board::kPIN_GPS_RX, board::kPIN_GPS_TX);
        if (gps.GetStatus() == false) {
            result = false;
#ifdef DEBUG_PORT
            debug.Put("\r\nFailed to init gps.");
#endif
        }

        return result;
    }

    static void AutoRead(void) {
        
        char * gpsString = NULL;
        //Flush GPS buffer.
        while ((gpsString = gps.Get()) != NULL) {
            /*Throw away stings*/
        }

        libpropeller::Scheduler acclScheduler(150 * 10);
        libpropeller::Scheduler gyroScheduler(100 * 10);
        libpropeller::Scheduler magnScheduler(25 * 10);
        libpropeller::Scheduler fuelScheduler(1); //10 second cycle
        libpropeller::Scheduler timeScheduler(10); //1 second cycle


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
            }
            if (timeScheduler.Run()) {
                ReadDateTime();
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
            
            if(set_clock == true){
                rtc.SetClock(set_year, set_month, set_day, set_hour, set_minute, set_second);
                set_clock = false;
            }
        }

    }

    static void ReadDateTime(void) {
        rtc.GetClock(year, month, day, hour, minute, second);
    }

    static void ReadGyro(void) {
        l3g.ReadGyro(gyro_x, gyro_y, gyro_z);
    }

    static void ReadAccl(void) {
        lsm.ReadAccl(accl_x, accl_y, accl_z);
    }

    static void ReadMagn(void) {
        lsm.ReadMagn(magn_x, magn_y, magn_z);
    }

    static void ReadFuel(void) {
        fuel_soc = fuel.GetStateOfCharge();
        fuel_voltage = fuel.GetVoltage();
        fuel_rate = fuel.GetChargeRate();
    }

    static void ReadBaro(void) {
        baro.Get(pressure, temperature, true);
    }

    /** This function serves requests for sensor data. When datalogging,
     * it automatically reads it's sensors.
     * 
     * Whenever sensor data is read it is PutIntoBuffer'ed.
     * 
     * Note that this function is blocking (won't return) until the sever is
     * killed with @a Stop(). So, @a Server should be called in it's 
     * own cog.
     */
    static void Server(void * temp) {
        AutoRead();
        killed = false; // Set flag to indicate done.
        cogstop(cogid());
    }
    
    

};

#ifdef SINGLE_TRANSLATION_UNIT
#include "Sensors.cpp"
#endif

#endif	/* PROPGCC_SRLM_SENSORS_H */


