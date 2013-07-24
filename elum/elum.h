/** Elum class to allow access to single Elums

@warning This should not be used for a "heartbeat" or watchdog type application,
since the LED will remain lit even if the program crashes or does some other
unexpected action.

@warning Not cog-safe: do not attempt to use from multiple cogs, even if they
don't use it simultaneously (it uses cog counters for flashing, so it won't work
on a different cog).

@warning This object may use the cog counters.
 * @TODO(SRLM): In which situations?

 * @TODO(SRLM): What are the hardware requirements?
 * 
Some parts based on Pins.h by David Michael Betz.
Some parts based on an example by Tracey Allen.

@author SRLM (srlm@srlmproductions.com)
 */

#include <stdio.h>
#include <stdint.h>
#include <propeller.h>


#ifndef SRLM_PROPGCC_ELUM_H_
#define SRLM_PROPGCC_ELUM_H_

//TODO(SRLM): Get rid of this min.
static inline int32_t Min__(int32_t a, int32_t b) {
    return a < b ? a : b;
}

class Elum {
public:

        /**
     * Use these constants to set the displayed LED color:
     * Max8819::RED
     * Max8819::GREEN
     */
    enum elumColor {
        RED, GREEN
    };

    enum patternType {
        kSingleSlow, kSingle, kSingleSyncopated, kDouble, kTriple, kManyFast, kJitterFast
    };


    /** Initialize the Elum class.

    @param RedPin the pin that the RED led is sunk to
    @param GreenPin the pin that the GREEN led is sunk to
    @param ButtonPin: the pin to make input to read the button. The button should
                      have a pullup, so that it is NC=1, and active low.
     */
    Elum(int RedPin, int GreenPin, int ButtonPin);

    Elum() {
    }

    void Start(int RedPin, int GreenPin, int ButtonPin);

    inline ~Elum() {
        Off();
    }

        /**
    @return true if button is pressed, false if it is not.
     */
    inline bool GetButton() {
        return (INA & b_mask) == 0;
    }

    inline void Slowclock(void) {
        _clockfreq = 20000;
    }
    void On(elumColor whichcolor);
    void Off(void);


    /**
    Set the LED color to flash.
     @example 
        elum.Flash(Elum::GREEN, 1000, 750); //Set Green to turn on for 750ms, and off for 250ms

    @param color     Elum::RED or Elum::GREEN
    @param period_ms The frequency that the color is flashed
    @param flash_ms  The duration of the displayed color. Must be less than period_ms
     */

    void Flash(int color, int period_ms, int flash_ms);




    /**
    Alternate in the following pattern:

    Single Slow: 5, 10, 0          (RG RG RG RG ...) ~13 seconds / cycle
    Single Slow Reverse: 5, 10, 50 (GR GR GR GR ...) ~13 seconds / cycle
    Single: 20, 60,  0             (G R G R G R ...)
    Single Syncopated: 20, 60, 50  (RG GR RG GR ...)
    Double: 20, 100, 0             (R R G G R R ...)
    Triple: 10, 50, 50             (G G GR R RG ...)
    Many Fast: 20, 400, 0          (G ... R ...    ) Fast flickers, repeat each color ~ 10 times ~3 seconds / cycle
    Jitter Fast: 200, 300, 0       (GGRR GGRR   ...) Really fast 

     */

    void Pattern(patternType pattern);


    /** Fades both LEDs in and out.

    @param frequency The rate at which to flash the combined LEDs, in units of
                     0.1Hz/LSb
     */
    void Fade(int frequency);


private:
    int pin_r;
    int pin_g;
    int _clockfreq;
    unsigned int b_mask;
    int Fraction(int Y, int X, int B);
    int Pwm(int Pin, int Period_ms, int Flash_ms);

    void Pattern(int tRed, int tGreen, int phsGreen);
};



#endif // SRLM_PROPGCC_ELUM_H_







