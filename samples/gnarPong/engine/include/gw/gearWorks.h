/*************************************************************************\
 Engine: gearWorks
 File: gearWorks.h
 Author: Zachry Thayer
 Description: include and global definition file
\*************************************************************************/

#ifndef _GEAR_WORKS_H
#define _GEAR_WORKS_H

#include <pspkernel.h>

#include "./gwTimer.h"
#include "./gwVram.h"
#include "./gwRender.h"
#include "./gwTexture.h"
#include "./gwModel.h"

bool gwRunning;

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

public:	
	
	static void init(){
		setupCallbacks();
		gwRunning = true;
	}
	
	static void exitGame(){
		gwRunning = false;
		sceKernelExitGame();
	}
	
	static bool isRunning(){
		return gwRunning;
	}
	
};

#endif
