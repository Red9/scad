#include <stdio.h>
#include "main.define.cpp"




int main(void){
	printf("Hello, World!\n");
	
#ifdef OUTPUTHIGH
	printf("Output High!\n");
#else
	printf("Output Low...\n");
#endif
	
	return 0;
}
