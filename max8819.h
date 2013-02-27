/* Max8819.h - Max8819 class to allow access to single pins

Copyright (c) 2012 David Michael Betz

Permission is hereby granted, free of charge, to any person obtaining a copy of this software
and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

*/

#include <stdint.h>
#include <propeller.h>


/**
@todo Add support (is it neccessary?) For the "timer fault" condition (figure 3,
page 18 of datasheet) when CHG outputs a 2Hz square wave.
*/
class Max8819 {
  private:
    uint32_t cen_mask;
    uint32_t chg_mask;
    uint32_t en_mask;
    uint32_t dlim1_mask;
    uint32_t dlim2_mask;
  public:
  
/**
Turns the power on, and turns off charging.
*/
Max8819(int CEN_pin, int CHG_pin, int EN_pin, int DLIM1_pin, int DLIM2_pin);
~Max8819();

/** Turn the power system on. Holds on until Off() is called.
*/
void On();

/** Turn the power system off.

@warning includes Propeller power, so this had better be the last thing in the
program.
*/
void Off();

/**

@todo If SetCharge is set to off, and it's plugged in, what does GetCharge return?
@returns the charging status of the battery. True indicates charging, false
         indicates no charging.
 
*/
bool GetCharge();

/**
Sets the charge rate. Valid constants are:
-OFF
-LOW
-MEDIUM
-HIGH
Note: The rate is set even if there is currently no external power, and stays in
effect until next called (possibly during the connection of external power).

With a 3kOhm CISET resistor, we have the following charge rates:
-LOW    == 95   mA
-MEDIUM == 475  mA
-HIGH   == 1000 mA

From the datasheet:
It is not necessary to limit the charge current based on the capabilities of
the expected AC-toDC adapter or USB/DC input current limit, the system load, or
thermal limitations of the PCB. The IC automatically lowers the charging current
as necessary to accommodate for these factors.

@param rate a constant specifying the rate of charge.
 */
void SetCharge(int rate);


enum{HIGH, MEDIUM, LOW, OFF};
};

/**
@warning pmic->SetCharge(Max8819::HIGH); //TODO: There is some sort of bug where this *must* be in the code, otherwise it causes a reset.
*/

inline Max8819::Max8819(int CEN_pin, int CHG_pin, int EN_pin, int DLIM1_pin, int DLIM2_pin) 
	: cen_mask(1 << CEN_pin), chg_mask(1 << CHG_pin), en_mask(1 << EN_pin), 
		dlim1_mask(1 << DLIM1_pin), dlim2_mask(1 << DLIM2_pin){
    DIRA |= cen_mask;   // Set to output
    DIRA |= en_mask;    // Set to output
    DIRA |= dlim1_mask; // Set to output
    DIRA |= dlim2_mask; // Set to output
    DIRA &= ~chg_mask;  // Set to input

    On();
    SetCharge(OFF);
}



inline Max8819::~Max8819(){
}


inline void Max8819::On(){
    OUTA |= en_mask;
}


inline void Max8819::Off(){
    OUTA &= ~en_mask;
}

inline bool Max8819::GetCharge(){
    return (INA & chg_mask) == 0;
}

inline void Max8819::SetCharge(int rate){
	if( rate == OFF ){
		OUTA &= ~cen_mask; //Turn off master switch
        OUTA |= dlim1_mask;//1
        OUTA |= dlim2_mask;//1

    }
    else if(rate == HIGH){
    	OUTA |= cen_mask;
        OUTA &= ~dlim1_mask; //0
        OUTA &= ~dlim2_mask; //0

    }
    else if(rate == MEDIUM){
    	OUTA |= cen_mask;
        OUTA &= ~dlim1_mask; //0
        OUTA |= dlim2_mask;  //1
    }
    else{ //rate == LOW
    	OUTA |= cen_mask;
        OUTA |= dlim1_mask; //1
        OUTA &= ~dlim2_mask;//0
    }
}


