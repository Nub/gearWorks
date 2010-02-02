TARGET = pong
OBJS = main.o intraFont.o

INCDIR = 
CFLAGS = -G0 -Wall -O0 -g -ffast-math
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS=-lgw -lstdc++ -lpspgum -lpspgu -lpspctrl -lpsprtc -lpng -lz -lm

EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Pong
BUILD_PRX = 1

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

logBuild:
	gwBuildManager -b