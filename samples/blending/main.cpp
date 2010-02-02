#include <pspdebug.h>
#include <pspkernel.h>
#include "../../../gearWorks/gearWorks.h"
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <math.h>

PSP_MODULE_INFO("Blending Sample", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);


gwTexture *lightmap, *background;

gwRect light = {0,0,200,200};
gwRect screen = {0,0,480,272};
float t = 0;
unsigned int c;

gwTimer_callback updateFrame(void *d){
	gwTimer *timer = reinterpret_cast<gwTimer*>(d);
	t += timer->deltaTime();
	if(t > 6.28) t = 0;
	light.x = 100 + sinf(t) * 50;
	light.y = 80 + cosf(t) * 50;
	return NULL;
}

gwTimer_callback renderFrame(void *pData){	
	gwRender *r = reinterpret_cast<gwRender*>(pData);
	r->start();
	r->toTexture(lightmap);
	r->clearScreen();

	r->ellipseGradient(&light, 32, 0xff000000|((int)((sinf(t)*0.5 + 0.5)*255)<<16)|((int)((sinf(t)*0.5 + 0.5)*255)<<8)|((int)((sinf(t)*0.5 + 0.5)*255)));
	
	r->toScreen();
	r->clearScreen();

	r->texture(background, &screen);

	sceGuBlendFunc(GU_ADD, GU_DST_COLOR, GU_SRC_COLOR, 0, 0);
	r->texture(lightmap, &screen);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	r->end();
	return NULL;
}

int main(int argc, void *argv[]){
	GearWorks::init();
	
	pspDebugScreenInit();
	pspDebugScreenEnableBackColor(0);
	
	gwRender *render = new gwRender(GW_PIXEL_FORMAT_8888);
	render->setClearColor(0);	
	
	gwTimer *timer = new gwTimer();
	timer->newEvent(1.f/60.f, (gwTimer_callback) updateFrame, timer);//update logic @ 60fps
	timer->newEvent(1.f/30.f, (gwTimer_callback) renderFrame, render);//render @ 30fps
		
	lightmap = new gwTexture(480,272, GW_PIXEL_FORMAT_5650,GW_VRAM);
	background = new gwTexture("background.png");

	
	while (GearWorks::isRunning()){
		timer->update();
	}
	
	delete render;
	delete lightmap;
	GearWorks::exitGame();
	return 1;
}
