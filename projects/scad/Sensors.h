#ifndef PROPGCC_SRLM_SENSORS_H
#define	PROPGCC_SRLM_SENSORS_H

#include <propeller.h>

#include "librednine/concurrentbuffer/concurrentbuffer.h"
#include "librednine/concurrentbuffer/pib.h"
#include "librednine/scheduler/scheduler.h"
#include "librednine/i2c/i2c.h"
#include "librednine/lsm303dlhc/lsm303dlhc.h"
#include "librednine/l3gd20/l3gd20.h"
#include "librednine/pcf8523/pcf8523.h"
#include "librednine/max17048/max17048.h"
#include "librednine/ms5611/ms5611.h"
#include "librednine/mtk3339/mtk3339.h"

/* Pin definitions */
#ifdef GAMMA
#include "scadgamma.h"
#elif BETA2
#include "scadbeta2.h"
#endif

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

/* Pin definitions */
#ifdef GAMMA
    static i2c bus1;
    static i2c bus2;
#elif BETA2
    static i2c bus;
#endif

    
    
    static LSM303DLHC * lsm;
    static L3GD20 * l3g;

    static PCF8523 * rtc;
    static MAX17048 * fuel;
    static MTK3339 * gps;
    static MS5611 * baro;

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


