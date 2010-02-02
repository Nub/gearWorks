TARGET_LIB = libgw.a
OBJS = gwTimer.o gwVram.o gwTexture.o gwRender.o gwObj.o

INCDIR = 
CFLAGS = -O2 -G0 -Wall -ffast-math
CXXFLAGS = $(CFLAGS) -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

LIBDIR =
LDFLAGS =
LIBS= 

PSPSDK=$(shell psp-config --pspsdk-path)
include $(PSPSDK)/lib/build.mak

install:
	mkdir -p $(PSPSDK)/include/gw
	cp -f *.h $(PSPSDK)/include/gw
	cp -f $(TARGET_LIB) $(PSPSDK)/lib/
	
uninstall:
	rm -r $(PSPSDK)/include/gw
	rm -f $(PSPSDK)/lib/$(TARGET_LIB)

manager:
	$(MAKE) -C gwBuildManager