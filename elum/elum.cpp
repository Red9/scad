#include "elum.h"

Elum::Elum(int32_t RedPin, int32_t GreenPin, int32_t ButtonPin) : b_mask(1 << ButtonPin)
{
    DIRA &= ~b_mask; //Set button to input

    pin_r = RedPin;
    pin_g = GreenPin;

    DIRA |= (1 << pin_r);
    DIRA |= (1 << pin_g);

    _clockfreq = CLKFREQ;
}




void Elum::Flash(int32_t Whichpin, int32_t Period_ms, int32_t Flash_ms)
{
    CTRA = (CTRB = 0); //Stop counters while updating

    if (Whichpin == RED)
    {
        OUTA |= 1 << pin_g; //Make second pin high
        Pwm(pin_r, Period_ms, Flash_ms);
    }
    else if(Whichpin == GREEN)
    {
        OUTA |= 1 << pin_r; //Make second pin high
        Pwm(pin_g, Period_ms, Flash_ms);

    }
}


void Elum::Fade(int frequency){
//Source: http://forums.parallax.com/showthread.php/143483-Make-an-LED-pulse-(smoothly-go-on-and-off)-without-any-code-interaction?p=1160777#post1160777
	Off();
	CTRA = CTRB = 0; //Stop counters while updating.	
	FRQA = (((1000) << 2) * (0x40000000/(CLKFREQ/1000)))/1000;// 1000 Hz base frequency.
	CTRA = (0b00100000 << 23) + pin_r;
	FRQB = (((10000 + frequency) << 2) * (0x40000000/(CLKFREQ/1000)))/10000;
	CTRB = (0b00100000 << 23) + pin_g;
}


void Elum::Pattern(patternType pattern){
//enum patternType{kSingleSlow, kSingle, kSingleSyncopated, kDouble, kTriple, kManyFast, kJitterFast};
	Off();
	switch (pattern){
		case kSingleSlow:
			Pattern(5, 10, 0);
			break;
		case kSingle:
			Pattern(20, 60, 0);
			break;
		case kSingleSyncopated:
			Pattern(20, 60, 50);
			break;
		case kDouble:
			Pattern(20, 100, 0);
			break;
		case kTriple:
			Pattern(10, 50, 50);
			break;
		case kManyFast:
			Pattern(20, 400, 0);
			break;
		case kJitterFast:
			Pattern(300, 400, 0);
			break;
		default:
			Pattern(0, 0, 0);
	}
}
void Elum::Pattern(int tRed, int tGreen, int phsGreen){
	PHSB = (0x7FFFFFFF / 100 * phsGreen) << 1;
	FRQA = tRed;
	FRQB = tGreen;	
	CTRA = (0b00100000 << 23) + pin_r;
	CTRB = (0b00100000 << 23) + pin_g;
}



void Elum::On(elumColor whichColor)
{

    CTRA = (CTRB = 0);
    if(whichColor == RED)
    {
        OUTA |= (1<<pin_g);
        OUTA &= ~(1<<pin_r);
    }
    else
    {
        OUTA &= ~(1<<pin_g);
        OUTA |= (1<<pin_r);
    }
}

void Elum::Off(void)
{
    CTRA = (CTRB = 0);
	OUTA &= ~(1 << pin_r);
	OUTA &= ~(1 << pin_g);

}

int Elum::Pwm(int Pin, int Period_ms, int Flash_ms)
{
    int	Phsx, Frqx;
    FRQA = (FRQB = 0);
    Flash_ms = ((Period_ms / 2) - (Min__(Flash_ms, (Period_ms / 2))));
    Phsx = Fraction(Flash_ms, Period_ms, 32);
    Frqx = Fraction(1, ((_clockfreq / 1000) * Period_ms), 32);
    PHSB = (PHSA + Phsx);
    FRQA = (FRQB = Frqx);
    _clockfreq = CLKFREQ;
    OUTA &= ~(1<<Pin);
    CTRA = (0x10000000 + Pin);
    CTRB = (0x10000000 + Pin);
    return Phsx;
}

int Elum::Fraction(int Y, int X, int B)
{
    int32_t F = 0;
    {
        int32_t _idx__0000;
        _idx__0000 = B;
        do
        {
            Y = (Y << 1);
            F = (F << 1);
            if (Y >= X)
            {
                Y = (Y - X);
                (F++);
            }
            _idx__0000 = (_idx__0000 + -1);
        }
        while (_idx__0000 >= 1);
    }
    return F;
}
