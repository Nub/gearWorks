#include <pspdebug.h>
#include <pspkernel.h>
#include <gw/gearWorks.h>

#include <math.h>

extern "C"{
#include "intraFont.h"
}
	
PSP_MODULE_INFO("Pong", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

gwRect bounds = {2,2,478,270};//playing area

gwRect ball = {148,10,10,10};

float masterSpeed = 20;

float ballSpeed = 0.075 * masterSpeed;
float ballSpeedIncrement = 0.01 * masterSpeed;
float ballAngle[2] = {-0.5,1};

float paddleSpeed = 0.09 * masterSpeed;
gwRect player = {2,2,5,50};
gwRect ai = {473,2,5,50};

int pScore = 0;
int aScore = 0;

gwTexture *offScreen, *accumBuffer;
intraFont *ltn;

static inline int randomNum(int min, int max)
{
	return min + rand() % max;
}

gwRect shrink = {1,1,478,270};
gwRect grow = {-1,-1,482,274};
gwRect screen = {0,0,480,272};


gwTimer_callback renderFrame(void *pData){
	gwRender *render = reinterpret_cast<gwRender*>(pData);
	render->start();
	render->toTexture(accumBuffer);
	render->texture(offScreen, &screen, 0.0f, 200);

	render->ellipse(&ball,32, ((0xFF << 24)|((0xFF & randomNum(50,255)) << 16)|((0xFF & randomNum(50,255)) << 8)|((0xFF & randomNum(50,255)))));	
	render->rectangle(&player,0xFF22BB00);
	render->rectangle(&ai,0xFF2200BB);

	
	render->toTexture(offScreen);
	render->clearScreen();
	render->texture(accumBuffer, &shrink);
	intraFontSetStyle(ltn, 1.0f,0xFFFFFFFF,((0xFF << 24)|((0xFF & randomNum(50,255)) << 16)|((0xFF & randomNum(50,255)) << 8)|((0xFF & randomNum(50,255)))),INTRAFONT_ALIGN_CENTER);
	intraFontPrintf(ltn, 240, 20, "%d                   %d", pScore, aScore);

	
	render->toScreen();
	//render->clearScreen();
	render->texture(offScreen, &shrink);
	
	render->rectangle(&player,0xFF00FF00);
	render->rectangle(&ai,0xFF0000FF);
	render->ellipse(&ball,32);	
	
	
	render->end();
}



int main(int argc, void *argv[]){
	GearWorks::init();
	pspDebugScreenInit();

	gwRender *render = new gwRender(GW_PIXEL_FORMAT_8888);
	//pspDebugScreenSetColorMode(GW_PIXEL_FORMAT_5650);
	render->setClearColor(0xFFFFFFFF);
	render->setPrimitiveRenderMode(GW_FILL);
	
//	gwTimer *timer = new gwTimer();
//	timer->newEvent(1.f/60.f,(gwTimer_callback)renderFrame, render);

	offScreen = new gwTexture(480,272,GW_PIXEL_FORMAT_5650,GW_VRAM);
	accumBuffer = new gwTexture(480,272,GW_PIXEL_FORMAT_5650,GW_VRAM);

	intraFontInit();
	ltn = intraFontLoad("flash0:/font/ltn1.pgf",0);
	if(!ltn)
		sceKernelExitGame();

	
	while (GearWorks::isRunning()) {
	//	timer->update();
		
	//	pspDebugScreenSetXY(0,0);
	//	pspDebugScreenPrintf("FPS: %f\tPlayer:%d\tAI:%d", timer->fps(),pScore,aScore);
		//pspDebugScreenSetOffset(512*272*2);
		
		renderFrame(render);
		
		ball.x += ballAngle[0] * ballSpeed;
		ball.y += ballAngle[1] * ballSpeed;
		
		if(ball.x > 480){// player scored
			//ballAngle[0] *= -1;
			pScore ++;
			ball.x = 148;
			ball.y = 10;
			ballSpeed = 0.075 * masterSpeed;
		}
		if(ball.x+3 < -ball.w){//ai scored
			//ballAngle[1] *= -1;
			aScore ++;
			ball.x = 148;
			ball.y = 10;
			ballSpeed = 0.075 * masterSpeed;
		}
		
		if(ball.y+3 > bounds.h)
			ballAngle[1] *= -1;
		if(ball.y+3 < bounds.y)
			ballAngle[1] *= -1;
		
		//paddle collision
		if(ball.x+3 < player.x + player.w && ballAngle[0] < 0 && ball.x + 13 > player.x){
			if(ball.y + 13 > player.y && ball.y + 3 < player.y + player.h){
				ballSpeed += ballSpeedIncrement;
				ballAngle[1] = sinf((((ball.y+13)-(player.y+(player.h/2)))/50.f)*3.14f);
				ballAngle[0] = cosf(ballAngle[1]);
			}
		}
		
		//paddle collision
		if(ball.x + 13 > ai.x && ballAngle[0] > 0 && ball.x + 3 < ai.x + ai.w){
			if(ball.y + 13 > ai.y && ball.y + 3 < ai.y + ai.h){
				ballSpeed += ballSpeedIncrement;
				ballAngle[1] = sinf((((ball.y+13)-(ai.y+(ai.h/2)))/50.f)*3.14f);
				ballAngle[0] = -cosf(ballAngle[1]);
				
			}
		}
		
		
		//Ai
		if(ball.x > 240 && ballAngle[0] > 0){
			if(ball.y > ai.y + 10)
				ai.y += paddleSpeed;
			if(ball.y+ball.h < ai.y+ai.h - 10)
				ai.y -= paddleSpeed;
			
		}
		//Player ai
		if(ball.x < 240 && ballAngle[0] < 0){
			if(ball.y > player.y + 10)
				player.y += paddleSpeed;
			if(ball.y+ball.h < player.y+player.h - 10)
				player.y -= paddleSpeed;
			
		}

	}
	
//	delete timer;
	GearWorks::exitGame();
	return 1;
}
