/* 
 * File:   Sensors.h
 * Author: clewis
 *
 * Created on April 25, 2013, 5:45 PM
 */

#ifndef PROPGCC_SRLM_SENSORS_H
#define	PROPGCC_SRLM_SENSORS_H

#include <propeller.h>

#include <stdlib.h>


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


    volatile int fuel_soc;
    volatile int fuel_rate;
    volatile int fuel_voltage;

    int year, month, day, hour, minute, second;


    int pressure, temperature;

    int gyro_x, gyro_y, gyro_z;
    int accl_x, accl_y, accl_z;
    int magn_x, magn_y, magn_z;

    static const int SensorTypeLength = 10;

    enum SensorType {
        kAccl, kGyro, kMagn, kFuel, kTime, kBaro, kGPS,
        kAccl2, kGyro2, kMagn2, kNone
    };

    Sensors(void);
    virtual ~Sensors(void);

    /** Setup to start the sensors Server. Must be called once (and only
     *  once).
     *
     */
    void init(void);
    
    /** This function serves requests for sensor data. When not datalogging,
     * it spins and waits for requests for specific data. When datalogging,
     * it automatically reads it's sensors.
     * 
     * In both cases, whenever sensor data is read it is PutIntoBuffer'ed.
     * 
     * Note that this function is blocking (won't return) until the sever is
     * killed with @a KillServer(). So, @a Server should be called in it's 
     * own cog.
     */
    void Server(void);
    
    /** Kill the server if running.
     * 
     */
    void KillServer(void);


    /** Request updated information about a specific sensor type.
     * 
     * @param type The sensor to read.
     * 
     */
    void Update(SensorType type, bool new_putIntoBuffer);


    /**
     * 
     * @param timeout the timeout in milliseconds
     */
    //bool GetLock(int timeout = -1);
    //void ReturnLock(void);


    void SetAutomaticRead(bool new_value);


private:

    i2c * bus;
    LSM303DLHC * lsm;
    L3GD20 * l3g;

    PCF8523 * rtc;
    MAX17048 * fuel;
    MTK3339 * gps;
    MS5611 * baro;

#ifdef EXTERNAL_IMU
    i2c * bus2;
    LSM303DLHC * lsm2;
    L3GD20 * l3g2;

    int gyro2_x, gyro2_y, gyro2_z;
    int accl2_x, accl2_y, accl2_z;
    int magn2_x, magn2_y, magn2_z;
#endif

    int lockID;

    volatile bool killed;

    volatile bool readControl[SensorTypeLength];
    volatile bool automaticRead;

    volatile bool controlledIntoBuffer;

    void AutoRead(void);
    void ControlledRead(void);

    void AddScales(void);

    void ReadDateTime(void);

    void ReadGyro(void);
    void ReadAccl(void);
    void ReadMagn(void);

    void ReadFuel(void);
    void ReadBaro(void);

    void ReadGyro2(void);
    void ReadAccl2(void);
    void ReadMagn2(void);





};

#endif	/* PROPGCC_SRLM_SENSORS_H */


