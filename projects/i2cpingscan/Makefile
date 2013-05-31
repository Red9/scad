#This makefile is based on here:
# http://www.borngeek.com/2010/05/06/automatic-dependency-generation/

SHELL = /bin/bash

ifndef MODEL
MODEL = cmm
endif

#If no root given, make best guess
ifndef ROOT
ROOT=$(shell root=$$(pwd); while [ "$$(basename $$root)" != "propgcc" ]; do root=$$(dirname $$root); done; echo $$root"/")
endif

ifndef BC
	BC=debug
endif

UNITYPATH = ${ROOT}unity/
GCCUTILITIESPATH = ${ROOT}gccutilities/
BOARDCONFIGURATIONPATH = ${ROOT}boardconfiguration/


INCLUDE = -I${UNITYPATH} -I${GCCUTILITIESPATH} -I${BOARDCONFIGURATIONPATH} -I../

#-Wa,-alh,-L  == output to STDOUT the generated assembly, with C interspersed

DEBUGFLAGS = #-Wall -Wa,-alh,-L #-save-temps #-ftime-report -fmem-report #-Q

CXX = propeller-elf-g++
CLD = propeller-elf-ld
#-Wall

OPTIMIZEFLAGS = -Os -fno-exceptions -ffunction-sections -fdata-sections -fno-rtti -m32bit-doubles -mfcache -fpermissive
CXXFLAGS =  -m${MODEL} ${INCLUDE} ${DEBUGFLAGS} ${DEFINEFLAGS} ${OPTIMIZEFLAGS} -std=gnu++0x -fno-strict-aliasing
CLDFLAGS = -Wl,--gc-sections #-Wl,--print-gc-sections #-Wl,--verbose #-Wl,--script=${GCCUTILITIESPATH}main.ld 



#DEFINES = #-DMY_SYMBOL
#INCPATH = #-I../some/path

#ifeq ($(BC),debug)
#	CFLAGS += -g3
#else
#	CFLAGS += -O2
#endif

DEPDIR=$(BC)/deps
OBJDIR=$(BC)/objs

# Build a list of the object files to create, based on the .cpps we find
OTMP = $(patsubst %.cpp,%.o,$(wildcard *.cpp))

# Build the final list of objects
OBJS = $(patsubst %,$(OBJDIR)/%,$(OTMP))

# Build a list of dependency files
DEPS = $(patsubst %.o,$(DEPDIR)/%.d,$(OTMP))

Debug: init $(OBJS)
	$(CXX) $(CXXFLAGS) $(CLDFLAGS) -o main.elf $(OBJS)

init:
	mkdir -p $(DEPDIR)
	mkdir -p $(OBJDIR)

# Pull in dependency info for our objects
-include $(DEPS)

# Compile and generate dependency info
# 1. Compile the .cpp file
# 2. Generate dependency information, explicitly specifying the target name
# 3. The final three lines do a little bit of sed magic. The following
#    sub-items all correspond to the single sed command below:
#    a. sed: Strip the target (everything before the colon)
#    b. sed: Remove any continuation backslashes
#    c. fmt -1: List words one per line
#    d. sed: Strip leading spaces
#    e. sed: Add trailing colons
$(OBJDIR)/%.o : %.cpp
	$(CXX) $(DEFINES) $(CXXFLAGS) -o $@ -c $<
	$(CXX) -MM -MT $(OBJDIR)/$*.o $(DEFINES) $(CXXFLAGS) \
		$*.cpp > $(DEPDIR)/$*.d
	@cp -f $(DEPDIR)/$*.d $(DEPDIR)/$*.d.tmp
	@sed -e 's/.*://' -e 's/\\\\$$//' < $(DEPDIR)/$*.d.tmp | fmt -1 | \
		sed -e 's/^ *//' -e 's/$$/:/' >> $(DEPDIR)/$*.d
	@rm -f $(DEPDIR)/$*.d.tmp

clean:
	rm -fr debug/*
	rm -fr release/*
