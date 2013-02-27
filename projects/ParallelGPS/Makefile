#Significant help in regards to the ".depend" section comes from:
# http://stackoverflow.com/questions/2481269/how-to-make-simple-c-makefile

ifndef MODEL
MODEL = cmm
endif

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

CXX = propeller-elf-g++
CLD = propeller-elf-ld
CXXFLAGS = -m${MODEL} -Wall -Os -m32bit-doubles -mfcache -fno-exceptions -fno-rtti -fpermissive -ffunction-sections -fno-strict-aliasing

Debug: cleanDebug ${OBJS}
	${CXX} ${CXXFLAGS} -o main.elf ${OBJS}

depend: .depend

.depend: ${SRCS}
	rm -f ./.depend
	${CXX} ${CXXFLAGS} --MM $^>>./.depend;

cleanDebug:
	${RM} *.o *.elf .depend *.map *.rawmap a.out
	
clean: cleanDebug

include .depend
