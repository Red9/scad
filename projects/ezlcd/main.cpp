#include <propeller.h>
#include "serial.h"
#include "c++-alloc.h"

#include "stdlib.h"

const int kPIN_USER_4 = 21; //RX from LCD
const int kPIN_USER_5 = 22; //TX to LCD


Serial * lcd = nullptr;

const int charWidth = 14;
const int charHeight = 24;
const int lcdDelay = 60;

/*
Status Variables
*/
volatile bool datalogging = false;
volatile bool sdPresent = false;

volatile int fuel_soc = 99;
volatile int fuel_rate = -127;
volatile int fuel_voltage = 4127;

volatile int year, month, day, hour, minute, second;

volatile int gyro_x, gyro_y, gyro_z, accl_x, accl_y, accl_z;

volatile int SDBufferFree, debugBufferFree;


void setXY(int x, int y){
	waitcnt(CLKFREQ/lcdDelay + CNT);
	lcd->Put("XY %d %d\r\n", x, y);
	waitcnt(CLKFREQ/lcdDelay + CNT);
}

void setXYChar(int x, int y){
	setXY(x*charWidth, y*charHeight);
}

void drawLine(int x0, int y0, int x1, int y1){
	setXY(x0 * charWidth + charWidth/2, y0 * charHeight + charHeight/2);
	lcd->Put("LINE %d %d\r\n", x1 * charWidth + charWidth/2, y1 * charHeight + charHeight/2);
	waitcnt(CLKFREQ/lcdDelay + CNT);
}


void clearArea(int x0, int y0, int x1, int y1){
//	setXY(x0 * charWidth + charWidth/2, y0 * charHeight + charHeight/2);
	setXY(x0 * charWidth, y0 * charHeight);
	lcd->Put("COLOR black\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	lcd->Put("BOX %d %d F\r\n", (x1-x0+1) * charWidth, (y1-y0+1) * charHeight);
	waitcnt(CLKFREQ/lcdDelay + CNT);
	lcd->Put("COLOR white\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	
}

void lcdInit(void){
	lcd = new Serial;
	lcd->Start(kPIN_USER_4, kPIN_USER_5, 115200);

	lcd->Put("CLS black\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	
	lcd->Put("COLOR white\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	
	lcd->Put("FONT KIN24\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	
	lcd->Put("LINEWIDTH 3\r\n");
	waitcnt(CLKFREQ/lcdDelay + CNT);
	
	//Print Compile information
	setXYChar(0,0);
	lcd->Put("PRINT \"Compiled:\"\r\n");
	setXYChar(0, 1);
	lcd->Put("PRINT \"%s\"\r\n", __DATE__);
	setXYChar(0, 2);
	lcd->Put("PRINT \"%s\"\r\n", __TIME__);
	
	
	
	drawLine(0,3, 11,3);
	drawLine(11,3, 11, 0);
	
	drawLine(11,3, 21,3);
	drawLine(21,0, 21,3);
	
	drawLine(21,3, 26,3);
	
	drawLine(0,7, 7,7);
	drawLine(7,3, 7,7);
	
	drawLine(7,7, 14,7);
	drawLine(14,3, 14,7);
	
	drawLine(14,7, 17,7);
	drawLine(17,7, 17,8);
}



void updateScreen(void){
//	lcd->Put("CLS black\r\n");
	
	
	//Print Battery information
	clearArea(12,0, 20,2);
	setXYChar(12, 0);
	lcd->Put("PRINT \"Batt %3d%%\"\r\n", fuel_soc);
	setXYChar(15, 1);
	lcd->Put("PRINT \"%d.%3dv\"\r\n", fuel_voltage/1000, fuel_voltage%1000);
	setXYChar(12, 2);
	lcd->Put("PRINT \"%3d.%d%%/hr\"\r\n", fuel_rate/10, abs(fuel_rate)%10);
	
	
	//Print Buffer information
	clearArea(22,0, 26,2);
	setXYChar(22,0);
	lcd->Put("PRINT \"SD%3d\"\r\n", SDBufferFree);
	setXYChar(22,1);
	lcd->Put("PRINT \"DG%3d\"\r\n", debugBufferFree);

	
	
	//Print Gyro information
	clearArea(0,4, 6,6);
	setXYChar(0,4);
	lcd->Put("PRINT \"x%6d\"\r\n", gyro_x);
	setXYChar(0,5);
	lcd->Put("PRINT \"y%6d\"\r\n", gyro_y);
	setXYChar(0,6);
	lcd->Put("PRINT \"z%6d\"\r\n", gyro_z);
	
	
	//Print Accl information
	clearArea(8,4, 13,6);
	setXYChar(8,4);
	lcd->Put("PRINT \"x%5d\"\r\n", accl_x);
	setXYChar(8,5);
	lcd->Put("PRINT \"y%5d\"\r\n", accl_y);
	setXYChar(8,6);
	lcd->Put("PRINT \"z%5d\"\r\n", accl_z);
	
	
	//Print time information
	clearArea(0,8, 16,8);
	setXYChar(0,8);
	lcd->Put("PRINT \"%2d/%2d/%2d %2d:%2d:%2d\"\r\n", year, month, day, hour, minute, second);
	
}


int main(void){
	lcdInit();


	for(fuel_soc = 0; fuel_soc < 115; fuel_soc++){
	
		updateScreen();
		waitcnt(CLKFREQ/2 + CNT);
	}
	
	

	
}
