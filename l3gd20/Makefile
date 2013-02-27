ifndef MODEL
MODEL = cmm
endif

SRCS=$(wildcard *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

UNITYPATH = ../unity/

#I2CPATH = ../i2c/
#I2CCPP = ${I2CPATH}i2cbase.cpp ${I2CPATH}i2c.cpp

INCLUDE = -I${UNITYPATH} -I../ #-I${I2CPATH}

CXX = propeller-elf-g++
CLD = propeller-elf-ld
#-Wall
CXXFLAGS = -m${MODEL} ${INCLUDE}  -Os -m32bit-doubles -mfcache -fno-exceptions -fno-rtti -fpermissive -ffunction-sections -fno-strict-aliasing
CLDFLAGS = -Wl,--script=main.ld -Wl,--gc-sections #-Wl,--print-gc-sections

Debug:
	${CXX} ${CXXFLAGS} ${CLDFLAGS} -o main.elf ${SRCS} ${UNITYPATH}unity.cpp #${I2CCPP}

cleanDebug: clean

clean:
	${RM} *.o *.elf *.map *.rawmap a.out *.generate.*

