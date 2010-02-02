TARGET = objTest
GEAR_WORKS_OBJS = ../../../gearWorks/gwTimer.o ../../../gearWorks/gwVram.o ../../../gearWorks/gwRender.o ../../../gearWorks/gwTexture.o ../../../gearWorks/gwObj.o
OBJS = $(GEAR_WORKS_OBJS) pgeFont.o main.o
INCDIR = 
CFLAGS = -G0 -Wall -O2 -g -ffast-math -DGW_VERBOSE
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= -lstdc++ -lpspgum_vfpu -lpspvfpu -lpspgu -lfreetype -lpspctrl -lpsprtc -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP

PSP_EBOOT_ICON = icon.png
PSP_EBOOT_PIC1 = pic.png
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

logBuild:
	gwBuildManager -b