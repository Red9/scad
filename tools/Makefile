#
# The following parameters can be defined:
# ROOT: the path to the Propeller GCC folder (propgcc, most likely)
# MODEL: the memory model to use.
# DEFINEFLAGS: ... ???

# If Unit tests are being run, the #define UNIT_TEST flag is set.

ifndef MODEL
MODEL = cmm
endif

#If no root given, make best guess
ifndef ROOT
ROOT=../../
#ROOT=$(shell root=$$(pwd); while [ "$$(basename $$root)" != "propgcc" ]; do root=$$(dirname $$root); done; echo $$root"/")
endif

ifndef LIBPROPELLER
LIBPROPELLER=../../
endif

ifndef DEFINEFLAGS
DEFINEFLAGS =
endif


#ifndef OUTPUTELF
#OUTPUTELF = main.elf
#endif

#Note the "string" version of *.cpp: that's so that the gcc compiler evaluates
#the wildcard, instead of make. This is because unit testing generates a .cpp
#file with the ruby command, and wildcard evaluates before it's generated.
#Have to do the wildcard *.S because otherwise gcc complains if there are not
#any .S files in the directory (if you pass it a *.S).
SRCS=main.cpp $(wildcard *.S) #$(wildcard *.cpp) $(wildcard *.S)
OBJS=$(subst .cpp,.o,$(SRCS))

UNITYPATH = ${ROOT}unity/
GCCUTILITIESPATH = ${ROOT}gccutilities/
BOARDCONFIGURATIONPATH = ${ROOT}boardconfiguration/
LIBREDNINE = ../../

INCLUDE = -I${GCCUTILITIESPATH} -I${BOARDCONFIGURATIONPATH} -I${LIBREDNINE} -I${LIBPROPELLER}

#-Wa,-alh,-L  == output to STDOUT the generated assembly, with C interspersed



DEBUGFLAGS = -Wall #-Wa,-alh,-L#-pedantic #-fstack-usage #-save-temps #-ftime-report -fmem-report #-Q

CXX = /opt/parallax/bin/propeller-elf-g++
CLD = propeller-elf-ld
ELFSIZE = /opt/parallax/bin/propeller-elf-size
ELFHUBSIZE = /opt/parallax/bin/propeller-elf-image-size

#-Wall

INLINEFLAGS = #-fno-default-inline

OPTIMIZEFLAGS =  -Os -ffunction-sections -fdata-sections -pipe -fno-exceptions -fno-rtti -m32bit-doubles -mno-fcache -fpermissive -fno-strict-aliasing
CXXFLAGS = -m${MODEL} ${INCLUDE} ${INLINEFLAGS} ${DEBUGFLAGS} ${DEFINEFLAGS} ${OPTIMIZEFLAGS} -std=gnu++0x
CLDFLAGS = -Wl,--gc-sections #-Wl,--print-gc-sections #-Wl,--verbose #-Wl,--script=${GCCUTILITIESPATH}main.ld 


release: beta2 gamma
	
beta2: buildbeta2
	rm -f main.elf
	ln -s main.beta2.elf main.elf
	${ELFSIZE} main.beta2.elf
	${ELFHUBSIZE} main.beta2.elf
	
gamma: buildgamma
	rm -f main.elf
	ln -s main.gamma.elf main.elf
	${ELFSIZE} main.gamma.elf
	${ELFHUBSIZE} main.gamma.elf
	
buildbeta2:
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -D BETA2 -D SINGLE_TRANSLATION_UNIT -o main.beta2.elf ${SRCS} ${LIBPROPELLER}libpropeller/compile_tools/statics/*
	
buildgamma:
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -D GAMMA -D SINGLE_TRANSLATION_UNIT -o main.gamma.elf ${SRCS} ${LIBPROPELLER}libpropeller/compile_tools/statics/*
	


cleanDebug: clean

clean:
	${RM} *.o *.map *.rawmap a.out *.generate.* *.d *.P *.elf



