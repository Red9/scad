#include <stdio.h>

//__attribute__((fcache))
int function(char * A, char * B, int stringIndex){
	int i = 0;
	
	printf("\r\n Useless line...");
	
	while(B[stringIndex] >= '0' and B[stringIndex] <= '9'){
		A[i++] = B[stringIndex];
//		printf("+%c+", B[stringIndex]);				
		stringIndex++;
	}
	
	printf("\r\n Useless line...");
	
	return i;
}


void test_1(void){
	char bufferA[10];
	char bufferB[] = "Hel987654lo";
	int i = function(bufferA, bufferB, 3);
    printf("\r\nHello World! i == %i", i);
    printf("\r\nBufferA = '0x%x 0x%x 0x%x 0x%x 0x%x 0x%x'\r\n",
    	bufferA[0], bufferA[1], bufferA[2], bufferA[3], bufferA[4], bufferA[5]);
}    

void test_2(void){
	char bufferA[10];
	char bufferB[] = "Hel987654lo";
	int i = function(bufferA, bufferB, 3);
    printf("\r\nHello World! i == %i", i);
    printf("\r\nBufferA = '0x%x 0x%x 0x%x 0x%x 0x%x 0x%x'\r\n",
    	bufferA[0], bufferA[1], bufferA[2], bufferA[3], bufferA[4], bufferA[5]);
}    

void test_3(void){
	char bufferA[10];
	char bufferB[] = "Hel987654lo";
	int i = function(bufferA, bufferB, 3);
    printf("\r\nHello World! i == %i", i);
    printf("\r\nBufferA = '0x%x 0x%x 0x%x 0x%x 0x%x 0x%x'\r\n",
    	bufferA[0], bufferA[1], bufferA[2], bufferA[3], bufferA[4], bufferA[5]);
}    

void test_4(void){
	char bufferA[10];
	char bufferB[] = "Hel987654lo";
	int i = function(bufferA, bufferB, 3);
    printf("\r\nHello World! i == %i", i);
    printf("\r\nBufferA = '0x%x 0x%x 0x%x 0x%x 0x%x 0x%x'\r\n",
    	bufferA[0], bufferA[1], bufferA[2], bufferA[3], bufferA[4], bufferA[5]);
}    

void test_5(void){
	char bufferA[10];
	char bufferB[] = "Hel987654lo";
	int i = function(bufferA, bufferB, 3);
    printf("\r\nHello World! i == %i", i);
    printf("\r\nBufferA = '0x%x 0x%x 0x%x 0x%x 0x%x 0x%x'\r\n",
    	bufferA[0], bufferA[1], bufferA[2], bufferA[3], bufferA[4], bufferA[5]);
}    

int main()
{	
	for(int i = 0; i < 10; i++){
		test_1();
		test_2();
		test_3();
		test_4();
		test_5();
	}
	
	
    return 0;
}
