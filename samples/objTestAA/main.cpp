#include <pspdebug.h>
#include <pspkernel.h>
#include "../../../gearWorks/gearWorks.h"
#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <math.h>

PSP_MODULE_INFO("ObjTest", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

static inline float random (const float Min, const float Max)
{
	return (rand () * (((Max) - (Min)) * (1.0f / (float)RAND_MAX)) + (Min));
}


ScePspFVector3 eye = {0,7,10};
ScePspFVector3 up = {0,1,0};
ScePspFVector3 center = {0,0,0};

gwObj *obj;
ScePspFVector3 objScale = {2,2,2};
gwTexture *screen;

int aa = 2;

gwRect view = {0,0,240,127};
gwRect fullScreen = {0,0,480,272};


float fps;

gwTimer_callback renderFrame(void *pData){	
	gwRender *render = reinterpret_cast<gwRender*>(pData);
	render->start();
	render->clearScreen();
	
	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumLookAt(&eye, &center, &up);
	
	sceGumMatrixMode(GU_MODEL);
#define AA 1
	
#ifdef AA
		view.w = fullScreen.w/aa;
		view.h = fullScreen.h/aa;
	
	for(int y = 0; y < aa; y ++)
		for(int x = 0; x < aa; x ++){
			view.x = x * view.w;
			view.y = y * view.h;
			render->toTexture(screen, &view);
			render->clearScreen();
#endif
			sceGumPushMatrix();
			sceGumScale(&objScale);
			obj->render();
			sceGumPopMatrix();
#ifdef AA		
			render->toScreen();
			render->texture(screen, &view);
		}
#endif
	
	pspDebugScreenSetOffset((int)gwVramRelativePointer(render->getFrameBuffer()));
	render->end();
	
	pspDebugScreenSetXY(0,0);
	pspDebugScreenPrintf("FPS:%f", fps);
	return NULL;
}

gwTimer_callback updateFrame(void *d){
	gwTimer *timer = reinterpret_cast<gwTimer*>(d);
	float time = timer->elapsedTime();
	eye.x = sinf(time) * 5.f;
	eye.y = sinf(time) * 5.f + 2.f;

	return NULL;
}


int main(int argc, void *argv[]){
	GearWorks::init();
	
	pspDebugScreenInit();
	pspDebugScreenEnableBackColor(0);
	
	gwRender *render = new gwRender(GW_PIXEL_FORMAT_8888);
	render->setClearColor(0);
	render->start();
	
	sceGumMatrixMode(GU_PROJECTION);
	sceGumLoadIdentity();
	sceGumPerspective(45.0f, 480.0f/272.0f, 1.0f, 32.f);

	sceGumMatrixMode(GU_VIEW);
	sceGumLoadIdentity();
	sceGumLookAt(&eye, &center, &up);
	
	sceGumMatrixMode(GU_MODEL);
			
	sceGuEnable(GU_LIGHTING);
	sceGuEnable(GU_LIGHT0);
	
	ScePspFVector3 pos = { 2,2,2 };
	sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
	sceGuLightColor(0,GU_DIFFUSE,0xFFffffff);
	sceGuLightColor(0,GU_SPECULAR,0xFFFFFFFF);
	sceGuLightAtt(0,0.f,0.5f,0.f);
	
	sceGuSpecular(10.0f);
	sceGuAmbient(0xff000000);
	render->end();

	obj = new gwObj("./ship.obj");
	gwTexture *tex = new gwTexture("./ship.png");
	obj->bindTexture(tex);
	
	gwTimer *timer = new gwTimer();
	timer->newEvent(1.f/30.f, (gwTimer_callback) renderFrame, render);//render @ 30fps
	timer->newEvent(1.f/60.f, (gwTimer_callback) updateFrame, timer);
	
	screen = new gwTexture(480, 272, GW_PIXEL_FORMAT_5650, GW_VRAM);
	
	while (GearWorks::isRunning()){
		timer->update();
		fps = timer->fps();
	}
	
	
	delete timer;
	GearWorks::exitGame();
	return 1;
}
