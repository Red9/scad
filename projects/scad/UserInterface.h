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

    UserInterface();

    void Init(const int kPinLedWhite, const int kPinLedRed, const int kPinButton);

    void DisplayState(int lastFuel);
    void SetState(DeviceState new_state);
    
    bool CheckButton(void);
    
    /** 
     *  Should call @CheckButton() before calling this function.
     * 
     * @return The duration of the most recent (or current) button press in ms.
     */
    int GetButtonPressDuration(void);

    void ClearButtonPressDuration(void);

private:
    Pin ledWhite, ledRed, button;

    volatile DeviceState previousState;
    Scheduler * displayDeviceState;
    
    
    static const bool kUseCTRA = true;
    
    static const int kFastBlink = 20;
    static const int kSlowBlink = 8;
    
    int buttonDuration;
    Stopwatch buttonTimer;
    
    DeviceState state;
    
    
    int dataloggingBlinkRate(const int kFuelSoc);
    
    
    
    

};

#endif	// REDNINE_USERINTERFACE_H_

