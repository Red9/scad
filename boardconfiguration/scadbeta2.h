
/**

*/

#ifndef SRLM_PROPGCC_BOARD_H__
#define SRLM_PROPGCC_BOARD_H__

namespace board{
	const int kPIN_I2C_SCL = 0;
	const int kPIN_I2C_SDA = 1;

	const int kPIN_LSM303DLHC_INT1 = 2;
	const int kPIN_LSM303DLHC_INT2 = 3;
	const int kPIN_LSM303DLHC_DRDY = 4;

	const int kPIN_LEDG           = 5;
	const int kPIN_MAX8819_EN123  = 6;
	const int kPIN_LEDR           = 7;
	const int kPIN_BUTTON         = 8;

	const int kPIN_SD_CD  = 9;
	const int kPIN_SD_DO  = 10;
	const int kPIN_SD_CLK = 11;
	const int kPIN_SD_DI  = 12;
	const int kPIN_SD_CS  = 13;

	const int kPIN_MAX8819_CHG = 14;
	const int kPIN_MAX8819_CEN = 15;
	const int kPIN_MAX8819_DLIM1 = 16;
	const int kPIN_MAX8819_DLIM2 = 17;


	const int kPIN_USER_1 = 18;
	const int kPIN_USER_2 = 19;
	const int kPIN_USER_3 = 20;
	const int kPIN_USER_4 = 21;
	const int kPIN_USER_5 = 22;
	const int kPIN_USER_6 = 23;


	const int kPIN_GPS_TX = 24; //Tx from the Propeller
	const int kPIN_GPS_RX = 25; //Rx to the Propeller
	const int kPIN_GPS_FIX = 26;

	const int kPIN_PCF8523_SQW = 27;

	const int kPIN_EEPROM_SCL = 28;
	const int kPIN_EEPROM_SDA = 29;

	const int kPIN_USB_TX = 30; //Tx from the Propeller
	const int kPIN_USB_RX = 31; //Rx to the Propeller
	

#ifdef EXTERNAL_IMU
	const int kPIN_I2C_SCL_2 = kPIN_USER_5;
	const int kPIN_I2C_SDA_2 = kPIN_USER_6;
#endif

	const int kPIN_BLUETOOTH_TX = kPIN_USER_2;
	const int kPIN_BLUETOOTH_RX = kPIN_USER_1;
	const int kPIN_BLUETOOTH_CTS = kPIN_USER_3;
	const int kPIN_BLUETOOTH_CONNECT = kPIN_USER_4;
}


#endif // SRLM_PROPGCC_BOARD_SCADBETA2__
