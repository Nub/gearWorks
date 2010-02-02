#include <pspdebug.h>
#include <pspkernel.h>
#include "../../../gearWorks/gearWorks.h"
#include <pspgu.h>
#include <pspprof.h>

PSP_MODULE_INFO("Render Test", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

gwTexture *test,*test2;

gwRect fullScreen = {0,0,480,272};

gwTimer_callback renderFrame(void *pData){
	gwRender *render = reinterpret_cast<gwRender*>(pData);
	render->start();
	render->clearScreen();
	//render->texture(test2, &fullScreen, 0.34f);
	for(int i = 0; i < 4; i ++);
	render->texture(test, &fullScreen, 0.34f);
	render->end();
}


void drawSprite(int sx, int sy, int width, int height, gwTextureS* source, int dx, int dy)
{
	//sceGuTexFunc(GU_TFX_REPLACE,GU_TCC_RGBA);
	//sceGuTexMode(source->format, 0, 0, source->swizzled);
	//sceGuTexImage(0, source->textureWidth, source->textureHeight, source->textureWidth, source->data);
	
//	float u = 1.0f / ((float)source->textureWidth);
	//float v = 1.0f / ((float)source->textureHeight);
	//sceGuTexScale(u, v);
	
//	sceGuDisable(GU_DEPTH_TEST);
	int j = 0;
	while (j < width) {
		struct BlitVertex {
			unsigned short u,v;
			short x,y,z;
		} *vertices = (struct BlitVertex*) sceGuGetMemory(2 * sizeof(struct BlitVertex));
		int sliceWidth = 64;
		if (j + sliceWidth > width) sliceWidth = width - j;
		vertices[0].u = sx + j;
		vertices[0].v = sy;
		vertices[0].x = dx + j;
		vertices[0].y = dy;
		vertices[0].z = 1;
		vertices[1].u = sx + j + sliceWidth;
		vertices[1].v = sy + height;
		vertices[1].x = dx + j + sliceWidth;
		vertices[1].y = dy + height;
		vertices[1].z = 1;
		sceGuDrawArray(GU_SPRITES, GU_TEXTURE_16BIT | GU_VERTEX_16BIT | GU_TRANSFORM_2D, 2, 0, vertices);
		j += sliceWidth;
	}
//	sceGuEnable(GU_DEPTH_TEST);
//	sceGuTexScale(1.0f, 1.0f);
}

int main(int argc, void *argv[]){
	GearWorks::init();
	pspDebugScreenInit();

	gwRender *render = new gwRender(GW_PIXEL_FORMAT_5650);
	pspDebugScreenSetColorMode(GW_PIXEL_FORMAT_5650);
	render->setClearColor(0xFF000000);
	render->setPrimitiveRenderMode(GW_FILL);
	
	test = new gwTexture("resources/test.png");
	test2 = new gwTexture("resources/test1.png", GW_VRAM, true);
		
//	gwTimer *timer = new gwTimer();
//	timer->newEvent(1.f/60.f, (gwTimer_callback) renderFrame, render);
	
	gwTextureS *test2s = test2->getTextureS();
	
	while (GearWorks::isRunning()) {
	//	timer->update();
		
	//	pspDebugScreenSetXY(0,1);
	//	pspDebugScreenPrintf("FPS: %f\ntexW: %d texH: %d Size: %d", timer->fps(),test2s->textureWidth,test2s->textureHeight, test2s->size/(1024));
		//pspDebugScreenSetOffset(512*272*2);
		render->start();
		render->clearScreen();
		render->texture(test2, &fullScreen);
	//	test2->activate();
	//	drawSprite(0,0,480,272, test2s, 0,0);
		//for(int i = 0; i < 4; i ++);
		//render->texture(test, &fullScreen);
		render->end();
	}
	
	
	//delete timer;
	gprof_cleanup();
	GearWorks::exitGame();
	return 1;
}
