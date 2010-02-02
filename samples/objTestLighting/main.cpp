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


gwVector3 eye = {0,7,10};
gwVector3 up = {0,1,0};
gwVector3 center = {0,0,0};

gwObj *obj, *objLow;
gwVector3 objScale = {1.5,1.5,1.5};
gwTexture *screen, *screen2, *screen3;

int aa = 2;

//gwRect fullScreen = {5,5,470,262};
gwRect fullScreenF = {5,5,470,262};

gwRect view = {0,0,240,127};
gwRect fullScreen = {0,0,480,272};

float fps;


gwTimer_callback renderFrame(void *pData){	
	gwRender *r = reinterpret_cast<gwRender*>(pData);
	r->start();
	
	r->matrixMode(GW_VIEW);
	r->identityMatrix();
	r->lookatMatrix(eye,center,up);
	
	r->matrixMode(GW_MODEL);
	
	r->toTexture(screen);
	r->clearScreen();
	
	//r->disable(GW_LIGHTING);
	
	r->pushMatrix();
	r->scale(objScale);
	objLow->render();
	r->popMatrix();
	
	//r->enable(GW_LIGHTING);
	
	r->convolution5x5(screen, screen2);
	
	r->toScreen();
	r->clearScreen();
	view.w = fullScreen.w/aa;
	view.h = fullScreen.h/aa;
	
/*	for(int y = 0; y < aa; y ++)
		for(int x = 0; x < aa; x ++){
			view.x = x * view.w;
			view.y = y * view.h;
			r->toTexture(screen3, &view);
			r->clearScreen();
			
			r->enable(GW_DITHER);*/
	
			r->pushMatrix();
			r->scale(objScale);
			obj->render();
			r->popMatrix();
	
		/*	r->disable(GW_DITHER);

	//	r->texture(screen2, &view);
			r->toScreen();
			r->texture(screen3, &view);
		}*/

	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0xFFFFFFFF, 0xFFFFFFFF);
	r->textureSprite(screen2, &fullScreen, &fullScreenF, 0 , 255);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

	pspDebugScreenSetOffset(r->getFrameBuffOffset());
	r->end();
	
	pspDebugScreenSetXY(0,0);
	pspDebugScreenPrintf("FPS:%f", fps);
	return NULL;
}
float t= 0;
gwTimer_callback updateFrame(void *d){
	gwTimer *timer = reinterpret_cast<gwTimer*>(d);
	t += 0.005f;
	if(t >= 6.28f) t = 0;
	eye.x = sinf(t) * 5.f;
	eye.z = cosf(t) * 5.f;
	eye.y = sinf(t/2) * 5.f;
	return NULL;
}


int main(int argc, void *argv[]){
	GearWorks::init();
	
	pspDebugScreenInit();
	pspDebugScreenEnableBackColor(0);
	
	gwRender *render = new gwRender(GW_PIXEL_FORMAT_8888);
	render->setClearColor(0);
	render->start();

	
	render->enable(GW_LIGHTING);
	render->enable(GW_LIGHT0);
	render->enable(GW_LIGHT1);

	
	ScePspFVector3 pos = { 2,2,2 };
	sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
	sceGuLightColor(0,GU_DIFFUSE,0xFFFFFFFF);
	sceGuLightColor(0,GU_SPECULAR,0xFFFFFFFF);
	sceGuLightAtt(0,0.f,0.8f,0.f);
	
	ScePspFVector3 pos2 = { 0,0,-3 };
	sceGuLight(1,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos2);
	sceGuLightColor(1,GU_DIFFUSE,0xFF0088FF);
	sceGuLightColor(1,GU_SPECULAR,0xFFFFFFFF);
	sceGuLightAtt(1,0.f,1.5f,0.f);
	
	sceGuSpecular(100.0f);
	sceGuAmbient(0xff000000);
	render->end();

	obj = new gwObj("./ship.obj");
	gwTexture *tex1 = new gwTexture("./ship.png");
	tex1->swizzle();
	tex1->toVram();
	obj->bindTexture(tex1);

	objLow = new gwObj("./shiplow.obj");
	gwTexture *tex = new gwTexture("./shiplow.png");
	tex->swizzle();
	tex->toVram();
	objLow->bindTexture(tex1);
	
	gwTimer *timer = new gwTimer();
	//timer->newEvent(1.f/30.f, (gwTimer_callback) renderFrame, render);//render @ 30fps
	timer->newEvent(1.f/60.f, (gwTimer_callback) updateFrame, timer);//update logic @ 60fps
	
	screen = new gwTexture(240, 136, GW_PIXEL_FORMAT_5650, GW_VRAM);
	screen2 = new gwTexture(240, 136, GW_PIXEL_FORMAT_5650, GW_VRAM);
	//screen3 = new gwTexture(480, 272, GW_PIXEL_FORMAT_5650, GW_VRAM);

	
	while (GearWorks::isRunning()){
		timer->update();
		fps = timer->fps();
		renderFrame(render);
	}
	
	
	delete timer;
	delete screen;
	delete screen2;
	delete obj;
	delete objLow;
	delete tex1;
	delete tex;
	delete render;
	GearWorks::exitGame();
	return 1;
}
