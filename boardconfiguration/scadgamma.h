
/**

*/

#ifndef SRLM_PROPGCC_BOARD_H__
#define SRLM_PROPGCC_BOARD_H__

namespace board{

	const int kPIN_BLUETOOTH_TX = 0;
	const int kPIN_BLUETOOTH_RX = 1;
	const int kPIN_BLUETOOTH_CTS = 2;
	const int kPIN_BLUETOOTH_CONNECT = 3;

	const int kPIN_I2C_SCL_2 = 4;
	const int kPIN_I2C_SDA_2 = 5;
	
	const int kPIN_MAX8819_EN123  = 6;
	
	const int kPIN_BUTTON         = 7;
	
	const int kPIN_MAX8819_DLIM2 = 8;
	const int kPIN_MAX8819_DLIM1 = 9;
	
	const int kPIN_MAX8819_CHG = 10;
	const int kPIN_MAX8819_CEN = 11;
	
	const int kPIN_USER_1 = 12;
	const int kPIN_USER_2 = 13;
	const int kPIN_USER_3 = 14;
	
	const int kPIN_GPS_TX = 15; //Tx from the Propeller
	const int kPIN_GPS_RX = 16; //Rx to the Propeller
	
	const int kPIN_SD_CS  = 17;
	const int kPIN_SD_DI  = 18;
	const int kPIN_SD_CLK = 19;
	const int kPIN_SD_DO  = 20;
	const int kPIN_SD_CD  = 21;
	
	const int kPIN_LEDW           = 22;
	const int kPIN_LEDR           = 23;
	
	const int kPIN_LSM303DLHC_INT1 = 24;
	const int kPIN_LSM303DLHC_INT2 = 25;
	
	const int kPIN_I2C_SDA_1 = 26;
	const int kPIN_I2C_SCL_1 = 27;

	const int kPIN_EEPROM_SCL = 28;
	const int kPIN_EEPROM_SDA = 29;

	const int kPIN_USB_TX = 30; //Tx from the Propeller
	const int kPIN_USB_RX = 31; //Rx to the Propeller
}

#endif // SRLM_PROPGCC_BOARD_H__

