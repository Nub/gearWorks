/*************************************************************************\
 Engine: gearWorks
 File: gearWorks.h
 Author: Zachry Thayer
 Description: include and global definition file
\*************************************************************************/

#ifndef _GEAR_WORKS_H
#define _GEAR_WORKS_H

#include <stdarg.h>
#include <pspkernel.h>

#include "./gwTimer.h"
#include "./gwVram.h"
#include "./gwRender.h"
#include "./gwTexture.h"
#include "./gwModel.h"
#include "./gwObj.h"

bool gwRunning;
bool gwLogFile;
#define GW_LOGFILE "gwLog.txt"

class GearWorks{
private:
	
	static int exitCallback(int arg1, int arg2, void *common) {
		gwRunning = false;
		return 0;
	}
	
	static int callbackThread(SceSize args, void *argp) {
		int cbid;
		
		cbid = sceKernelCreateCallback("gwExitCallback", exitCallback, NULL);
		sceKernelRegisterExitCallback(cbid);
		
		sceKernelSleepThreadCB();
		
		return 0;
	}
	
	static int setupCallbacks(void) {
		int thid = 0;
		
		thid = sceKernelCreateThread("gwCallbackThread", callbackThread, 0x11, 0xFA0, 0, 0);
		if(thid >= 0) {
			sceKernelStartThread(thid, 0, 0);
		}
		
		return thid;
	}

	static int gwStdOutHandle(const char *data, int len){
		if(gwLogFile){
			
			int fd = sceIoOpen(GW_LOGFILE, PSP_O_WRONLY|PSP_O_CREAT|PSP_O_APPEND, 0777);
			sceIoWrite(fd, data, len);
			sceIoClose(fd);
			
		}else{
			
			//sceIoWrite(1, data, len);
			printf("%s", data);
			
		}
		return 1;
	}
	
public:	
	
	static void init(){
		setupCallbacks();
		gwRunning = true;
		gwLogFile = false;
		
		//pspDebugInstallStdoutHandler(&gwStdOutHandle);
		
		#ifdef GW_VERBOSE
			printf("Gearworks Initialized\n");
		#endif
		
	}
	
	static void exitGame(){
		#ifdef GW_VERBOSE
			printf("Gearworks Shutting down\n");
		#endif
		gwRunning = false;
		sceKernelExitGame();
	}
	
	static bool isRunning(){
		return gwRunning;
	}
	
	static void log(char *format, ...){
		char buffer[256];
		va_list ap;
		
		va_start(ap, format);
		vsnprintf(buffer, 256, format, ap);
		va_end(ap);
		buffer[255] = 0;
		
		//printf(buffer);
		sceIoWrite(2, buffer, 255);
	}
	
};

#endif
