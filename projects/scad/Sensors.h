#ifndef PROPGCC_SRLM_SENSORS_H
#define	PROPGCC_SRLM_SENSORS_H

#include <propeller.h>

#include "concurrentbuffer.h"
#include "pib.h"
#include "scheduler.h"
#include "i2c.h"
#include "lsm303dlhc.h"
#include "l3gd20.h"
#include "pcf8523.h"
#include "max17048.h"
#include "ms5611.h"
#include "mtk3339.h"

#include "scadbeta2.h"

class Sensors {
public:
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






    static void Start(void);

    /** Kill the server if running.
     * 
     */
    static void Stop(void);

    /**
     * 
     * @param timeout the timeout in milliseconds
     */
    //bool GetLock(int timeout = -1);
    //void ReturnLock(void);


    //void SetAutomaticRead(bool new_value);
    static void PauseReading(void);
    static void ResumeReading(void);

    /**
     * @TODO: Make sure this thread safe!
     */
    static void AddScales(void);
private:

    static i2c bus;
    static LSM303DLHC * lsm;
    static L3GD20 * l3g;

    static PCF8523 * rtc;
    static MAX17048 * fuel;
    static MTK3339 * gps;
    static MS5611 * baro;

#ifdef EXTERNAL_IMU
    static i2c * bus2;
    static LSM303DLHC * lsm2;
    static L3GD20 * l3g2;

    static int gyro2_x, gyro2_y, gyro2_z;
    static int accl2_x, accl2_y, accl2_z;
    static int magn2_x, magn2_y, magn2_z;
#endif

    static volatile bool paused;

    static volatile bool killed;





    static const int stackSize = 176 + 100;
    static int stack[stackSize];

    /** Setup to start the sensors Server. Must be called once (and only
     *  once).
     *
     */
    static void init(void);



    static void AutoRead(void);

    static void ReadDateTime(void);

    static void ReadGyro(void);
    static void ReadAccl(void);
    static void ReadMagn(void);

    static void ReadFuel(void);
    static void ReadBaro(void);

    static void ReadGyro2(void);
    static void ReadAccl2(void);
    static void ReadMagn2(void);


    /** This function serves requests for sensor data. When datalogging,
     * it automatically reads it's sensors.
     * 
     * Whenever sensor data is read it is PutIntoBuffer'ed.
     * 
     * Note that this function is blocking (won't return) until the sever is
     * killed with @a Stop(). So, @a Server should be called in it's 
     * own cog.
     */
    static void Server(void * temp);
};

#endif	/* PROPGCC_SRLM_SENSORS_H */


