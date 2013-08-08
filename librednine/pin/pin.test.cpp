#include "unity.h"
#include "pin.h"
#include "c++-alloc.h"

//#include "propeller.h"

const int kPIN_LED_1 = 22;
const int kPIN_LED_2 = 23;


Pin * led = NULL;

void setUp(void) {
    led = new Pin(kPIN_LED_2);
}

void tearDown(void) {
    delete led;
    led = NULL;
}

void test_Nothing(void) {
    
    Pin bPin(kPIN_LED_1);
    
    
    led->pwm(10, true, &bPin);
    waitcnt(CLKFREQ*5 + CNT);
    led->pwm(0);
    waitcnt(CLKFREQ*5 + CNT);
    led->pwm(100, false, &bPin);
    led->pwm(0);
    
}