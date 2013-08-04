
/**
Functions used to interface to the ezLCD-301 smart touch LCD
 http://store.earthlcd.com/ezLCD-301
Not used because the LCD seemed to have some sort of hardware problem.

This code was used in the context of the SCAD data logger.

This code just treats the LCD as a big character display. It divides the screen
into a grid (TODO(SRLM): how big? roughly 27x8) of characters, and you can 
write text starting at a particular cell. You can also draw a line between two
cells (useful for making dividers).
*/


Serial * lcd = nullptr;
const int charWidth = 14;
const int charHeight = 24;
const int lcdDelay = 60; //60;






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

void DrawInitial(void){

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

	drawLine(17,7, 26,7); //Above free read cycles

}

void lcdInit(void){
	lcd = new Serial;
	lcd->Start(kPIN_USER_4, kPIN_USER_5, 115200);
	
//	DrawInitial();
}



void updateScreen(int proportion){
//	lcd->Put("CLS black\r\n");
	

	//Print Battery information
	clearArea(12,0, 20,2);
	setXYChar(12, 0);
	lcd->Put("PRINT \"Batt %3d%%\"\r\n", fuel_soc);
	setXYChar(15, 1);
	lcd->Put("PRINT \"%d.%3dv\"\r\n", fuel_voltage/1000, fuel_voltage%1000);
	setXYChar(12, 2);
	lcd->Put("PRINT \"%3d.%d%%/hr\"\r\n", fuel_rate/10, abs(fuel_rate)%10);

	//Print time information
	clearArea(0,8, 16,8);
	setXYChar(0,8);
	lcd->Put("PRINT \"%2d/%2d/%2d %2d:%2d:%2d\"\r\n", year, month, day, hour, minute, second);

	
	//Print the filename
	waitcnt(CLKFREQ/lcdDelay + CNT);
	clearArea(15,4, 26,4);
	setXYChar(15,4);
	lcd->Put("PRINT \"%s\"\r\n", currentFilename);
	waitcnt(CLKFREQ/lcdDelay + CNT);
	

	for(int j = 0; j < proportion; j++){
	
		
	
		//Print Buffer information and status information
		clearArea(22,0, 26,2);
		setXYChar(22,0);
		lcd->Put("PRINT \"SD%3d\"\r\n", SDBufferFree);
		setXYChar(22,1);
		lcd->Put("PRINT \"DG%3d\"\r\n", debugBufferFree);
		setXYChar(22,2);
		lcd->Put("PRINT \"STS%2d\"\r\n", currentState);



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
		
		//Print Free Read Cycles information
		clearArea(18,8, 26,8);
		setXYChar(18,8);
		lcd->Put("PRINT \"%4d<%4d\"\r\n", freeReadCycles, freeReadCyclesMax);
	}
}

void RunLCD(void * parameter){
	waitcnt(CLKFREQ*2 + CNT);
	for(;;){
		DrawInitial();
		freeReadCyclesMax = 0;
		for(int i = 0; i < 10; i++){
			updateScreen(10);
		}
	}
	cogstop(cogid());
}
