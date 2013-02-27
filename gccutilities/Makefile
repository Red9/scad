#
# The following two parameters can be defined:
# ROOT: the path to the Propeller GCC folder (propgcc, most likely)
# MODEL: the memory model to use.



ifndef MODEL
MODEL = cmm
endif

#If no root given, make best guess
ifndef ROOT
ROOT = ../
endif

SRCS=$(wildcard *.cpp) #$(wildcard *.S)
OBJS=$(subst .cpp,.o,$(SRCS))

UNITYPATH = ${ROOT}unity/
GCCUTILITIESPATH = ${ROOT}gccutilities/

INCLUDE = -I${UNITYPATH} -I${GCCUTILITIESPATH} -I../

#-Wa,-alh,-L  == output to STDOUT the generated assembly, with C interspersed



DEBUGFLAGS = #-Wa,-alh,-L #-save-temps #-ftime-report -fmem-report #-Q

CXX = propeller-elf-g++
CLD = propeller-elf-ld
#-Wall
CXXFLAGS = -m${MODEL} ${INCLUDE} ${DEBUGFLAGS} -Os -m32bit-doubles -mfcache -fno-exceptions -fno-rtti -fpermissive -ffunction-sections -fno-strict-aliasing -std=gnu++0x
CLDFLAGS = -Wl,--script=${GCCUTILITIESPATH}main.ld -Wl,--gc-sections #-Wl,--print-gc-sections

Debug:
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -o main.elf ${SRCS}
	
Unit:
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -o main.elf -D UNIT_TEST ${SRCS} ${UNITYPATH}unity.cpp

cleanDebug: clean

clean:
	${RM} *.o *.elf *.map *.rawmap a.out *.generate.*

