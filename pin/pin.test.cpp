#include "unity.h"
#include "pin.h"

//#include "propeller.h"

void setUp(void)
{

}

void tearDown(void)
{

}


// -----------------------------------------------------------------------------
// Number to Bin String
// -----------------------------------------------------------------------------
int shiftLeft(int param){
//	param = param << 1;
	__asm__ volatile (
		"shl %[param], #1\n\t"
	: /*outputs */
		[param] "+r" (param)
	: /*inputs */
//		[param] "+r" (param)
	
		
	);
	

//    __asm__ volatile (
//        "rcr %[pin], #1 wc\n\t"
//        "setpc %[value]"
//    : /* outputs */
//        [value] "+r" (value)
//    : /* inputs */
//        [pin] "r" (pin),
//    : /* no clobbered registers */
//    );


	return param;
}

int strlenASM(char * string){
	int i = 0;
	int t1;

	__asm__(
		"strloop: "
		"rdbyte %[t1], %[string]\n\t"
		"cmp %[t1], #0 wz\n\t"
		"if_nz add %[i], #1\n\t"
		"if_nz add %[string], #1\n\t"
		"if_nz brs #strloop"
		
	: /*outputs (+inputs) */
		[i]	     "+r" (i),
		[t1] "=&r" (t1)
	: /*inputs */
		[string] "r" (string)
					
	);
	return i;
}


char * dec_to_ascii(int input_num, char * string){
//					 2147483647
//	int decimal[] = {1000000000, 100000000, 10000000, 1000000, 100000, 10000, 1000, 100, 10, 1};
//	int * decimal2 = decimal;
	char * starting_string = string;
	int count, divisor;
	divisor = 0;
//	count = 0;
	
	if(input_num == 0){
		string[0] = '0';
		string[1] = '\0';
		return starting_string;
	}
	
	if(input_num < 0){
		input_num = -input_num;
		string[0] = '-';
		string++;
	}


                        
	__asm__ volatile(
            

            
            "brw	#start_							\n\t"
            
            "str_start_addr: "
            "long 0									\n\t"
            
            "divisor_array: "
            "long	1000000000						\n\t"
			"long	100000000						\n\t"
			"long	10000000						\n\t"
			"long	1000000							\n\t"
			"long	100000							\n\t"
			"long	10000							\n\t"
			"long	1000							\n\t"
			"long	100								\n\t"
			"long	10								\n\t"
			"long	1								\n\t"

			"start_: "
			"mvi	r0, #divisor_array				\n\t"   
			"nop									\n\t"
			
			
			"start_digit: "
            "rdlong %[divisor], r0					\n\t"
            "cmp	%[input_num], %[divisor] wc		\n\t"
	"if_c	 add	r0, #4							\n\t"
	"if_c	 brw	#start_digit					\n\t"

       		"nop									\n\t"

       		     
            "main_loop: "
            "rdlong %[divisor], r0					\n\t" /* Read the current decimal divisor (10, 1000, 10000, etc.)*/
            
            "mov	%[count], #0					\n\t" /*Clear counter*/
			
			"count_digit: "
			"cmp	%[input_num], %[divisor] wc		\n\t"
	"if_nc   sub	%[input_num], %[divisor]		\n\t"
	"if_nc   add	%[count], #1					\n\t"
	"if_nc   brw    #count_digit					\n\t"
	
			"add	%[count], #48					\n\t" /*Convert to ascii representation*/
			"wrbyte %[count], %[string]				\n\t" /*Store result*/
	
			"add	%[string], #1					\n\t" /*increment to next char address*/
			
			"cmp	%[divisor], #1 wz				\n\t" /*Are we at the last digit?*/
	"if_nz	 add	r0, #4							\n\t"
	"if_nz	 brw	#main_loop						\n\t"
	
			"mov	%[count], #0					\n\t" /*Null terminate */
			"wrbyte %[count], %[string]				\n\t"
         
	: /*outputs (+inputs) */
		[string] "+r" (string),
		[count] "+&r" (count),
		[divisor] "+&r" (divisor)
		
		/*[decimal_addr] "+m" (decimal),*/
		
	
	
	: /*inputs */
		[input_num] "r" (input_num)
		
	
	: /*clobber */
		"r0"
	
	
	);
	
	return starting_string;
}


void test_Inline(void){
	TEST_ASSERT_BITS(0xFFFFFFFF, 0b100, shiftLeft(0b010));
}

void test_InlineStrlen(void){
	char string [] = "Hello";
	
	//Add some timing information...
	//int start_cnt = CNT;
	int result = strlenASM(string);
	//int end_cnt = CNT;
	
	//printf("Total CNT==%d  ", end_cnt - start_cnt);
	
	TEST_ASSERT_EQUAL_INT(strlen(string), result);
}


void test_InlineStrlen1(void){
	char string [] = "Big String!";
	
	TEST_ASSERT_EQUAL_INT(strlen(string), strlenASM(string));
}

void test_InlineStrlen2(void){
	char string [] = "";
	
	TEST_ASSERT_EQUAL_INT(strlen(string), strlenASM(string));
}

void test_InlineDecToAscii(void){
	char string[20];
	dec_to_ascii(9, string);
	TEST_ASSERT_EQUAL_STRING("9", string);
}


void test_InlineDecToAscii1(void){
	char string[20];
	dec_to_ascii(4, string);
	TEST_ASSERT_EQUAL_STRING("4", string);
}


void test_InlineDecToAscii2(void){
	char string[20];
	dec_to_ascii(0, string);
	TEST_ASSERT_EQUAL_STRING("0", string);
}

void test_InlineDecToAscii3(void){
	char string[20];
	dec_to_ascii(38, string);
	TEST_ASSERT_EQUAL_STRING("38", string);
}

void test_InlineDecToAscii4(void){
	char string[20];
	dec_to_ascii(385853, string);
	TEST_ASSERT_EQUAL_STRING("385853", string);
}

void test_InlineDecToAscii5(void){
	char string[20];
	dec_to_ascii(901, string);
	TEST_ASSERT_EQUAL_STRING("901", string);
}

void test_InlineDecToAscii6(void){
	char string[20];
	dec_to_ascii(7000, string);
	TEST_ASSERT_EQUAL_STRING("7000", string);
}

void test_InlineDecToAscii7(void){
	char string[20];
	dec_to_ascii(-5935, string);
	TEST_ASSERT_EQUAL_STRING("-5935", string);
}














