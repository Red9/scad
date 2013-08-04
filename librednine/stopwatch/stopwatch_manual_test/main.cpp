

#include "stopwatch.h"
#include <propeller.h>
#include "serial.h"


int main(){
	Serial debug;
	debug.Start(31, 30, 115200);
	
	debug.Put("Stopwatch Manual Test.\r\n");

	while(true){
		debug.Put("Press any key to start.\r\n");
		debug.Get();
		Stopwatch sw;
		sw.Start();
		debug.Put("Press any key to stop.\r\nElapsed Time: \r\n");
		
		
		while(debug.Get(0) == -1){
			//debug.Put("\r       ");
			debug.PutFormatted("\r%dms    ", sw.GetElapsed());
		}
		debug.Put("\r\n---------------------\r\n\r\n");
		
	}

	return 0;
	
}
