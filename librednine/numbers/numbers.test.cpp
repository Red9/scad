#include "unity.h"
#include "numbers.h"

#include "propeller.h"

void setUp(void)
{

}

void tearDown(void)
{

}


// -----------------------------------------------------------------------------
// Number to Bin String
// -----------------------------------------------------------------------------

void test_BinReturnAddressMatchesGivenAddress(void){
	char string[40];
	char * return_addr = Numbers::Bin(0b110, 4, string);
	TEST_ASSERT_EQUAL_PTR(string, return_addr);
}

void test_BinZero(void){
	char string[40];
	TEST_ASSERT_EQUAL_STRING("0000", Numbers::Bin(0b0000, 4, string));
}

void test_BinPositiveNumber(void){
	char string[40];
	TEST_ASSERT_EQUAL_STRING("10100101", Numbers::Bin(0b10100101, 8, string));
}

void test_Bin32BitNumber(void){
	char string[40];
	TEST_ASSERT_EQUAL_STRING("10101010000000001111111111001100",
		Numbers::Bin(0b10101010000000001111111111001100, 32, string));
}

void test_BinNoMoreDigitsThanSpecified(void){
	char string[40];
	TEST_ASSERT_EQUAL_STRING("1111", Numbers::Bin(0b11111111, 4, string));
}

void test_BinInternalBuffer(void){
	TEST_ASSERT_EQUAL_STRING("01010", Numbers::Bin(0b01010, 5));
}




// -----------------------------------------------------------------------------
// Number to Hex String
// -----------------------------------------------------------------------------
void test_HexReturnAddressMatchesGivenAddress(void){
	char string [20];
	char * return_addr = Numbers::Hex(0x555, 5, string);
	TEST_ASSERT_EQUAL_PTR(string, return_addr);
}

void test_HexZero(void){
	char string[20];
	TEST_ASSERT_EQUAL_STRING("00", Numbers::Hex(0x0, 2, string));

}

void test_HexPositiveNumber(void){
	char string[20];
	TEST_ASSERT_EQUAL_STRING("FA467", Numbers::Hex(0xFA467, 5, string));
}

void test_Hex32BitNumber(void){
	char string[20];
	TEST_ASSERT_EQUAL_STRING("FEDCBA98", Numbers::Hex(0xFEDCBA98, 8, string));
}

void test_HexNoMoreDigitsThanSpecified(void){
	char string [20];
	TEST_ASSERT_EQUAL_STRING("250F", Numbers::Hex(0x90250F, 4, string));
}

void test_HexEachDigit(void){
	char string [20];
	TEST_ASSERT_EQUAL_STRING("01234567", Numbers::Hex(0x01234567, 8, string));
	TEST_ASSERT_EQUAL_STRING("89ABCDEF", Numbers::Hex(0x89ABCDEF, 8, string));
}

void test_HexInternalBuffer(void){
	TEST_ASSERT_EQUAL_STRING("DEF", Numbers::Hex(0xDEF, 3));
}

// -----------------------------------------------------------------------------
// Number to Dec String
// -----------------------------------------------------------------------------
void test_DecReturnAddressMatchesGivenAddress(void)
{
	char string [20];
	char * return_addr = Numbers::Dec(555, string);
	TEST_ASSERT_EQUAL_PTR(string, return_addr);
}

void test_DecZero(void)
{
	char string [20];
	TEST_ASSERT_EQUAL_STRING("0", Numbers::Dec(0, string));
}



void test_DecPositiveNumber(void)
{
	char string [20];
	TEST_ASSERT_EQUAL_STRING("542", Numbers::Dec(542, string));
}

void test_DecNegativeNumber(void)
{
	char string [20];
	
	//Add some timing information...
	int start_cnt = CNT;
	Numbers::Dec(-3258656, string);
	int end_cnt = CNT;
	
	printf("Total CNT==%d  ", end_cnt - start_cnt);
	
	TEST_ASSERT_EQUAL_STRING("-3258656", string);
	
}

void test_DecVeryLargeNumber(void)
{
	char string [20];
	TEST_ASSERT_EQUAL_STRING("2000000001", Numbers::Dec(2000000001, string));
}

void test_DecInternalBuffer(void)
{
	TEST_ASSERT_EQUAL_STRING("123456", Numbers::Dec(123456));
}

// -----------------------------------------------------------------------------
// Dec String to Number
// -----------------------------------------------------------------------------
void test_DecBasic(void){
	TEST_ASSERT_EQUAL_INT(1234, Numbers::Dec("1234"));
}

void test_DecNegativeBasic(void){
	TEST_ASSERT_EQUAL_INT(-987, Numbers::Dec("-987"));
}

void test_DecBigNumber(void){
	TEST_ASSERT_EQUAL_INT(9864753, Numbers::Dec("9864753"));
}

void test_DecNonZeroTerminator(void){
	char number [] = {'3', '1', '\n'};
	TEST_ASSERT_EQUAL_INT(31, Numbers::Dec(number, '\n'));
}

void test_DecZeroNumbers(void){
	TEST_ASSERT_EQUAL_INT(0, Numbers::Dec("0"));
	TEST_ASSERT_EQUAL_INT(0, Numbers::Dec("00000"));
}

void test_DecLeadingZeros(void){
	TEST_ASSERT_EQUAL_INT(135, Numbers::Dec("00135"));
}

void test_DecEmptyString(void){
	TEST_ASSERT_EQUAL_INT(0, Numbers::Dec(""));
}

void test_DecCharArray(void){
	char string[] = {'4', '2', 0};
	TEST_ASSERT_EQUAL_INT(42, Numbers::Dec(string));
}

// -----------------------------------------------------------------------------
// Reverse
// -----------------------------------------------------------------------------

void test_ReverseReturnAddressMatchesGivenAddress(void)
{
	char string [20];
	char * return_addr = Numbers::Reverse(string);
	TEST_ASSERT_EQUAL_PTR(string, return_addr);
}

void test_ReverseEmptyString(void)
{
	char string [20];
	string[0] = 0;
	TEST_ASSERT_EQUAL_STRING("", Numbers::Reverse(string));
}

void test_ReverseSimpleString(void)
{
	char string [] = "abcdef";
	TEST_ASSERT_EQUAL_STRING("fedcba", Numbers::Reverse(string));
}

// -----------------------------------------------------------------------------
// Assorted Tests
// -----------------------------------------------------------------------------
void test_DecToDec(void){
	TEST_ASSERT_EQUAL_INT(531, Numbers::Dec(Numbers::Dec(531)));
}

void test_DecToDec2(void){
	TEST_ASSERT_EQUAL_STRING("468", Numbers::Dec(Numbers::Dec("468")));
}

// -----------------------------------------------------------------------------
// DecDigits
// -----------------------------------------------------------------------------

void test_DecDigitsZero(void){
	TEST_ASSERT_EQUAL_INT(1, Numbers::DecDigits(0));}
void test_DecDigits1(void){
	TEST_ASSERT_EQUAL_INT(1, Numbers::DecDigits(5));}
void test_DecDigits2(void){
	TEST_ASSERT_EQUAL_INT(2, Numbers::DecDigits(55));}
void test_DecDigits3(void){
	TEST_ASSERT_EQUAL_INT(3, Numbers::DecDigits(555));}
void test_DecDigits4(void){
	TEST_ASSERT_EQUAL_INT(4, Numbers::DecDigits(5555));}
void test_DecDigits5(void){
	TEST_ASSERT_EQUAL_INT(5, Numbers::DecDigits(55555));}
void test_DecDigits6(void){
	TEST_ASSERT_EQUAL_INT(6, Numbers::DecDigits(555555));}
void test_DecDigits7(void){
	TEST_ASSERT_EQUAL_INT(7, Numbers::DecDigits(5555555));}
void test_DecDigits8(void){
	TEST_ASSERT_EQUAL_INT(8, Numbers::DecDigits(55555555));}
void test_DecDigits9(void){
	TEST_ASSERT_EQUAL_INT(9, Numbers::DecDigits(555555555));}
void test_DecDigits10(void){
	TEST_ASSERT_EQUAL_INT(10, Numbers::DecDigits(1555555555));}
void test_DecDigitsNegative1(void){
	TEST_ASSERT_EQUAL_INT(2, Numbers::DecDigits(-5));}
void test_DecDigitsNegative2(void){
	TEST_ASSERT_EQUAL_INT(3, Numbers::DecDigits(-55));}
void test_DecDigitsNegative3(void){
	TEST_ASSERT_EQUAL_INT(4, Numbers::DecDigits(-555));}
void test_DecDigitsNegative4(void){
	TEST_ASSERT_EQUAL_INT(5, Numbers::DecDigits(-5555));}
void test_DecDigitsNegative5(void){
	TEST_ASSERT_EQUAL_INT(6, Numbers::DecDigits(-55555));}
void test_DecDigitsNegative6(void){
	TEST_ASSERT_EQUAL_INT(7, Numbers::DecDigits(-555555));}
void test_DecDigitsNegative7(void){
	TEST_ASSERT_EQUAL_INT(8, Numbers::DecDigits(-5555555));}
void test_DecDigitsNegative8(void){
	TEST_ASSERT_EQUAL_INT(9, Numbers::DecDigits(-55555555));}
void test_DecDigitsNegative9(void){
	TEST_ASSERT_EQUAL_INT(10, Numbers::DecDigits(-555555555));}
void test_DecDigitsNegative10(void){
	TEST_ASSERT_EQUAL_INT(11, Numbers::DecDigits(-1555555555));}

// -----------------------------------------------------------------------------
void test_HexDigitsZero(void){
	TEST_ASSERT_EQUAL_INT(1, Numbers::HexDigits(0x0));}
void test_HexDigits1(void){
	TEST_ASSERT_EQUAL_INT(1, Numbers::HexDigits(0xF));}
void test_HexDigits2(void){
	TEST_ASSERT_EQUAL_INT(2, Numbers::HexDigits(0xFE));}
void test_HexDigits3(void){
	TEST_ASSERT_EQUAL_INT(3, Numbers::HexDigits(0xFED));}
void test_HexDigits4(void){
	TEST_ASSERT_EQUAL_INT(4, Numbers::HexDigits(0xFEDC));}
void test_HexDigits5(void){
	TEST_ASSERT_EQUAL_INT(5, Numbers::HexDigits(0xFEDCB));}
void test_HexDigits6(void){
	TEST_ASSERT_EQUAL_INT(6, Numbers::HexDigits(0xFEDCBA));}
void test_HexDigits7(void){
	TEST_ASSERT_EQUAL_INT(7, Numbers::HexDigits(0xFEDCBA9));}
void test_HexDigits8(void){
	TEST_ASSERT_EQUAL_INT(8, Numbers::HexDigits(0xFEDCBA98));}


























