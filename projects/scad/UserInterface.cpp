/* 
 * File:   UserInterface.cpp
 * Author: clewis
 * 
 * Created on August 7, 2013, 3:45 PM
 */

#include "UserInterface.h"

#include "libpropeller/serial/serial.h"
extern Serial * debug;

UserInterface::UserInterface() {
    currentState = kUnknown;
    //Make sure that we're keeping track of the last time display has been updated.
    displayDeviceState = new Scheduler(1); //0.1 Hz

}

void UserInterface::Init(const int kPinLedWhite, const int kPinLedRed, const int kPinButton) {
    ledWhite = Pin(kPinLedWhite);
    ledRed = Pin(kPinLedRed);
    button = Pin(kPinButton);


    ledWhite.high();
    ledRed.low();

    button.input();
}

bool UserInterface::CheckButton(void) {
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

int UserInterface::GetButtonPressDuration(void) {
    return buttonDuration;
}

void UserInterface::ClearButtonPressDuration(void){
    buttonDuration = 0;
}

void UserInterface::DisplayDeviceStatus(const DeviceState state, int lastFuel
        ) {

    //Exit if the state is the same, and we've updated the display recently.
    if (currentState == state && !displayDeviceState->Run()) {
        return;
    }

    if (lastFuel > 100) {
        lastFuel = 100;
    } else if (lastFuel < 0) {
        lastFuel = 0;
    }



    currentState = state;

    ledRed.pwm(0);
    ledWhite.pwm(0);


    if (currentState == kUnknownError) {
        ledRed.pwm(kFastBlink, kUseCTRA, &ledWhite);
    } else if (currentState == kNoSD) {
        ledRed.pwm(kSlowBlink, kUseCTRA, &ledWhite);
    } else if (currentState == kCharging) {
        if (lastFuel < 90) {//pmic->GetCharge() == true){
            //Fade LED in and out
            ledRed.high();
            ledWhite.low();
        } else {
            //Done charging status
            ledWhite.high();
            ledRed.low();
        }
    } else if (currentState == kDatalogging) {
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
    } else if (currentState == kPowerOn
            || currentState == kWaiting) {
        ledWhite.high();
        ledRed.low();
    } else {
        //If nothing else, then turn off LED.
        ledWhite.low();
        ledRed.low();
    }


}

int UserInterface::dataloggingBlinkRate(const int fuel) {
    return (-90 * fuel) / 100 + 100;

}

