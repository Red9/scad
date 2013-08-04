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
    MS5611(i2c * newbus) {
        bus = newbus;

        GetStatus();
        if (status == false) {
            return;
        }

        const char kPROMRead [] = {
            0b10100000, // 16 bit reserved for manufacturer
            0b10100010, // C1
            0b10100100, // C2
            0b10100110, // C3
            0b10101000, // C4
            0b10101010, // C5
            0b10101100, // C6
            0b10101110 // CRC
        };


        D1 = 0; // Pressure
        D2 = 0; // Temperature

        //Read PROM here
        int C[6];
        for (int i = 0; i < 6; i++) {
            char data[2];
            bus->Put(deviceBaro, kPROMRead[i + 1]);
            bus->Get(deviceBaro, data, 2);
            C[i] = data[0] << 8 | data[1];

        }
        SetC(C[0], C[1], C[2], C[3], C[4], C[5]);

        convertingTemperature = true;
        bus->Put(deviceBaro, kConvertD2OSR4096);

        newData = false;

        conversionValidCNT = CNT + (CLKFREQ * 9) / 1000;

    }

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
    bool Touch(void) {
        if (CNT < conversionValidCNT //Not ready, simple case
                || (CNT > 0x7fffFfff && conversionValidCNT < 0x7fffFfff)
                ) {
            return false;
        }

        //Read ADC on MS5611, and get whatever it was converting.
        char data[3];

        bus->Put(deviceBaro, kADCRead);

        bus->Get(deviceBaro, data, 3);
        int reading = ExpandReading(data);
        newData = true;

        conversionValidCNT = CNT + (CLKFREQ * 9) / 1000;


        if (convertingTemperature) {
            D2 = reading;
            //Set ADC to convert pressure
            bus->Put(deviceBaro, kConvertD1OSR4096);

            convertingTemperature = false;
            return false;
        } else {
            D1 = reading;
            //Set ADC to convert temperature
            bus->Put(deviceBaro, kConvertD2OSR4096);

            convertingTemperature = true;
            return true;
        }
    }

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
            const bool calibrationCalculation = true) {

        if (calibrationCalculation == true) {
            if (newData) {
                Calculate();
            }
            tPressure = pressure;
            tTemperature = temperature;
            newData = false;
        } else {
            tPressure = D1;
            tTemperature = D2;
        }
    }

    /**
     * @returns true if device is present and ready, false otherwise
     */
    bool GetStatus(void) {
        if (bus == NULL) {
            status = false;
        } else {
            status = bus->Ping(deviceBaro);
        }
        return status;
    }

    /**
    @warning untested! (How do I test this?)
    @return true if successfully reset, false otherwise. Takes 2.8ms to reload.
     */
    bool Reset(void) {
        return bus->Put(deviceBaro, kReset);
    }

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
            const int newC4, const int newC5, const int newC6) {
        C1 = ((int64_t) newC1) << 15;
        C2 = ((int64_t) newC2) << 16;
        C3 = (int64_t) newC3;
        C4 = (int64_t) newC4;

        C5 = newC5 << 8;
        C6 = newC6;
    }

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
            int & oldC4, int & oldC5, int & oldC6) {
        oldC1 = (int) (C1 >> 15);
        oldC2 = (int) (C2 >> 16);
        oldC3 = (int) C3;
        oldC4 = (int) C4;
        oldC5 = C5 >> 8;
        oldC6 = C6;
    }

    /**
     * @warning This function is for unit testing only!
     * @param newD1 The raw pressure value
     * @param newD2 The raw temperature value
     */
    void TEST_SetD(const int newD1, const int newD2) {
        D1 = newD1;
        D2 = newD2;
        newData = true;
    }

private:

    void Calculate(void) {
        //These equations are straight from the MS5611 datasheet.
        int dT = D2 - C5;
        temperature = 2000 + ((dT * C6) >> 23);

        int64_t T2 = 0;
        int64_t OFF2 = 0;
        int64_t SENS2 = 0;

        if (temperature < 2000) {

            int64_t dT64 = dT;

            T2 = (dT64 * dT64) >> 31;
            OFF2 = (5 * (temperature - 2000) * (temperature - 2000)) >> 1;
            SENS2 = OFF2 >> 1;

            if (temperature < -1500) { //Very low temperature
                OFF2 = OFF2 + (7 * (temperature + 1500) * (temperature + 1500));
                SENS2 = SENS2 + ((11 * (temperature + 1500) * (temperature + 1500)) >> 1);
            }
        }


        int64_t OFF = C2 + ((C4 * dT) >> 7);
        int64_t SENS = C1 + ((C3 * dT) >> 8);

        temperature = temperature - T2;
        OFF = OFF - OFF2;
        SENS = SENS - SENS2;

        pressure = (int) ((((((int64_t) D1) * SENS) >> 21) - OFF) >> 15);
    }

    int ExpandReading(const char data[]) {
        //MS5611 returns a 24 bit unsigned number.
        return data[0] << 16 | data[1] << 8 | data[2];
    }

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

};

#endif // SRLM_PROPGCC_MS5611_H_
