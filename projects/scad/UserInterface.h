#ifndef REDNINE_USERINTERFACE_H_
#define	REDNINE_USERINTERFACE_H_

#include "libpropeller/pin/pin.h"
#include "libpropeller/scheduler/scheduler.h"
#include "libpropeller/stopwatch/stopwatch.h"

class UserInterface {
public:

    enum DeviceState {
        kPowerOn, kUnknownError, kNoSD, kCharging, kDatalogging, kDone, kUnknown,
        kPowerOff, kWaiting
    };

    UserInterface() : displayDeviceState(1){
        previousState = kUnknown;
        //Make sure that we're keeping track of the last time display has been updated.
        //displayDeviceState = new Scheduler(1); //0.1 Hz

    }

    void Init(const int kPinLedWhite, const int kPinLedRed, const int kPinButton) {
        ledWhite = libpropeller::Pin(kPinLedWhite);
        ledRed = libpropeller::Pin(kPinLedRed);
        button = libpropeller::Pin(kPinButton);


        ledWhite.high();
        ledRed.low();

        button.input();
    }

    void UpdateDisplayState(int lastFuel) {

        //Exit if the state is the same, and we've updated the display recently.
        if (previousState == state && !displayDeviceState.Run()) {
            return;
        }

        if (lastFuel > 100) {
            lastFuel = 100;
        } else if (lastFuel < 0) {
            lastFuel = 0;
        }



        previousState = state;

        ledRed.pwm(0);
        ledWhite.pwm(0);


        if (previousState == kUnknownError) {
            ledRed.pwm(kFastBlink, kUseCTRA, &ledWhite);
        } else if (previousState == kNoSD) {
            ledRed.pwm(kSlowBlink, kUseCTRA, &ledWhite);
        } else if (previousState == kCharging) {
            if (lastFuel < 90) {//pmic->GetCharge() == true){
                //Fade LED in and out
                ledRed.low();
                ledWhite.high();
            } else {
                //Done charging status
                ledWhite.low();
                ledRed.low();
            }
        } else if (previousState == kDatalogging) {
            if (lastFuel > 25) {
                //Flash green LED
                //elum.Flash(Elum::GREEN, 1000,  * 10);

                ledWhite.pwm(dataloggingBlinkRate(lastFuel), kUseCTRA);
                ledRed.low();


            } else {
                //Flash red LED
                //Plus one for the 0 case.
                //elum.Flash(Elum::RED, 1000, (kFuelSoc + 1) * 9);
                ledRed.pwm(dataloggingBlinkRate(lastFuel), kUseCTRA);
                ledWhite.low();
            }
        } else if (previousState == kPowerOn
                || previousState == kWaiting) {
            ledWhite.high();
            ledRed.low();
        } else {
            //If nothing else, then turn off LED.
            ledWhite.low();
            ledRed.low();
        }


    }

    void SetState(DeviceState new_state) {
        state = new_state;
    }

    bool CheckButton(void) {
        bool buttonPress =
#ifdef GAMMA
                button.input();
#elif BETA2
                !button.input();
#endif

        if (buttonPress == true) {
            if (buttonTimer.GetStarted() == false) {
                buttonTimer.Start();
            }
            buttonDuration = buttonTimer.GetElapsed();
        } else {
            buttonTimer.Reset();
        }
        return buttonPress;
    }

    /** 
     *  Should call @CheckButton() before calling this function.
     * 
     * @return The duration of the most recent (or current) button press in ms.
     */
    int GetButtonPressDuration(void) {
        return buttonDuration;
    }

    void ClearButtonPressDuration(void) {
        buttonDuration = 0;
    }

private:
    libpropeller::Pin ledWhite, ledRed, button;

    volatile DeviceState previousState;
    libpropeller::Scheduler displayDeviceState;


    static const bool kUseCTRA = true;

    static const int kFastBlink = 20;
    static const int kSlowBlink = 8;

    int buttonDuration;
    libpropeller::Stopwatch buttonTimer;

    DeviceState state;


    int dataloggingBlinkRate(const int fuel) {
    return (-90 * fuel) / 100 + 100;

}





};

#endif	// REDNINE_USERINTERFACE_H_

