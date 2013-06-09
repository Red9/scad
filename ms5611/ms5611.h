#ifndef SRLM_PROPGCC_MS5611_H_
#define SRLM_PROPGCC_MS5611_H_

#include <propeller.h>
#include "i2c.h"

/** MS5611 Barometer interface
 * 
 * Provides a simple interface to the MS5611 barometer and temperature sensor. 
 * Uses the I2C interface to the sensor.
 * 
 * @author SRLM (srlm@srlmproductions.com)
 */


class MS5611 {
public:

    /** Create a new MS5611 Barometer instance
     * 
     * This resets and initializes the sensor, reads the PROM, and begins a 
     * conversion.
     * 
     * @a Touch() should be called no sooner than 8.5ms after MS5611 
     * initialization.
     * 
     * @param newbus The I2C bus to use.
     */
    MS5611(i2c * newbus);

    /** Keep the MS5611 running.
     * 
     * In general, must be called twice for every associated @a Get().
     * 
     * "Touches" the sensor to make sure that it keeps converting at the maximum pace.
     * 
     * @warning Undefined behavior if called more than every 8.5 ms.
     * 
     * Test results indicate that the Touch function takes the following amount 
     * of time, at 80MHz in:
     * + CMM -Os mode: 49056 cycles (~0.62ms)
     * + LMM -Os mode: 22624 cycles (~0.28ms)
     * @returns true when a new set of data is ready. If result is true, then 
     *  @a Get() should be called.
     */
    bool Touch(void);

    /** Get the most current readings from the MS5611 sensor.
     * 
     * Test results indicate that the Get function takes the following amounts 
     * of time, at 80MHz in when calibrationCalculation == true:
     * 
     * + CMM -Os mode: 48448 cycles (~0.62ms)
     * + LMM -Os mode: 13968 cycles (~0.17ms)
     * 
     * @warning Second order temperature compensation has not been tested! 
     * Particularly the very low temperature compensation!
     * @param tPressure The pressure, either raw or in units of 0.01 mBar
     * @param tTemperature The temperature, either raw or in units of 0.01C
     * @param calibrationCalculation Perform calculations to calibrate (they 
     * involve 64 bit integers, so they take a while).
     */
    void Get(int & tPressure, int & tTemperature,
            const bool calibrationCalculation = true);

    /**
     * @returns true if device is present and ready, false otherwise
     */
    bool GetStatus(void);

    /**
    @warning untested! (How do I test this?)
    @return true if successfully reset, false otherwise. Takes 2.8ms to reload.
     */
    bool Reset(void);

    /**
    Set the C PROM calibration constants.
    @param C1
    @param C2
    @param C3
    @param C4
    @param C5
    @param C6
     */
    void SetC(const int newC1, const int newC2, const int newC3,
            const int newC4, const int newC5, const int newC6);

    /** Get the C PROM calibration constants.
     * 
     * @param C1
     * @param C2
     * @param C3
     * @param C4
     * @param C5
     * @param C6
     */
    void GetC(int & oldC1, int & oldC2, int & oldC3,
            int & oldC4, int & oldC5, int & oldC6);

    /**
     * @warning This function is for unit testing only!
     * @param newD1 The raw pressure value
     * @param newD2 The raw temperature value
     */
    void TEST_SetD(const int newD1, const int newD2);

private:
    void Calculate(void);
    int ExpandReading(const char data[]);

    i2c * bus;

    // These variables are straight from the MS5611 datasheet.
    int64_t C1, C2, C3, C4;
    int C5, C6;

    int D1, D2;
    int temperature, pressure;

    bool newData;
    bool convertingTemperature;
    bool status;

    unsigned int conversionValidCNT;

    const static char deviceBaro = 0b11101110;
    const static char kConvertD1OSR4096 = 0x48; //D1 is the pressure value
    const static char kConvertD2OSR4096 = 0x58; //D2 is the temperature value
    const static char kADCRead = 0x00;
    const static char kReset = 0b00011110;
    const static char kPROMRead[];

};

#endif // SRLM_PROPGCC_MS5611_H_
