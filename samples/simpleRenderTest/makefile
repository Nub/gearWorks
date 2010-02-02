TARGET = renderTest
GEAR_WORKS_OBJS = ../../../gearWorks/gwTimer.o ../../../gearWorks/gwVram.o ../../../gearWorks/gwRender.o ../../../gearWorks/gwTexture.o
OBJS = $(GEAR_WORKS_OBJS) main.o gprof/prof.o gprof/mcount.o

INCDIR = 
CFLAGS = -G0 -Wall -O2 -pg -g -ffast-math
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lstdc++ -lpspgum -lpspgu -lpspctrl -lpsprtc -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Gear Works Render Test
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

logBuild:
	gwBuildManager -b