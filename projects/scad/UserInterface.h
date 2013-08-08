#ifndef USERINTERFACE_H
#define	USERINTERFACE_H

#include "librednine/pin/pin.h"
#include "librednine/scheduler/scheduler.h"

//#include "libpropeller/c++allocate/c++allocate.h"


class UserInterface {
public:

    enum DeviceState {
        kPowerOn, kUnknownError, kNoSD, kCharging, kDatalogging, kDone, kUnknown,
        kPowerOff, kWaiting
    };

    UserInterface();

    void Init(const int kPinLedWhite, const int kPinLedRed, const int kPinButton);

    void DisplayDeviceStatus(DeviceState state, int lastFuel);
    
    bool GetButton(void);


private:
    Pin ledWhite, ledRed, button;

    volatile DeviceState currentState;
    Scheduler * displayDeviceState;
    
    
    static const bool kUseCTRA = true;
    
    static const int kFastBlink = 20;
    static const int kSlowBlink = 8;
    
    int dataloggingBlinkRate(const int kFuelSoc);

};

#endif	/* USERINTERFACE_H */

