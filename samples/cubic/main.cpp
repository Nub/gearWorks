#include <pspdebug.h>
#include <pspkernel.h>
#include "../../../gearWorks/gearWorks.h"

#include "pgeFont.h"

#include <pspgu.h>
#include <pspgum.h>
#include <pspctrl.h>
#include <math.h>

PSP_MODULE_INFO("Cube Zombie", PSP_MODULE_USER, 0, 1);
PSP_MAIN_THREAD_ATTR(PSP_THREAD_ATTR_VFPU);
PSP_HEAP_SIZE_KB(20480);

gwPoint analog;
SceCtrlData pad;


gwRect fullScreen = {0,0,480,272};

gwObj *dude;
ScePspFVector3 dudeScale = {1,1,1};
ScePspFVector3 dudePos = {0,0.3,0};
ScePspFVector3 dudeSpeed = {0,0,0};
ScePspFVector3 dudeSpeedMax = {0.08f,0.0f,0.08f};
ScePspFVector3 dudeRot = {0,45,0};

gwObj *gun;
ScePspFVector3 gunScale = {1,1,1};
ScePspFVector3 gunPos = {-0.2,-0.2,0.4};
ScePspFVector3 gunRot = {0,0,0};

gwObj *enemy;
ScePspFVector3 enemyScale = {0.3,0.3,0.3};
const int enemyMax = 40;
int enemyC = 10;
ScePspFVector3 enemyPos[enemyMax];
ScePspFVector3 enemySpeed[enemyMax];
float enemySpeedMax = 0.06f;
float enemyRotY;
float lastWave;
float waveFreq = 2.3f;

int kills = 0;
int health = 5;

static inline float random (const float Min, const float Max)
{
	return (rand () * (((Max) - (Min)) * (1.0f / (float)RAND_MAX)) + (Min));
}

void releaseEnemy(){
	//int c = (int)random(2,8);
	// for(int i = 0; i <  c; i++){
		 float rrot = random(0,360);
		 for(int j = 0; j < enemyC; j++){
			 if(enemySpeed[j].x == 0 && enemySpeed[j].z == 0){
				 float sine = sinf(rrot);
				 float cosine = cosf(rrot);
				 enemyPos[j].x = dudePos.x + sine * random(20, 30);
				 enemyPos[j].z = dudePos.z + cosine * random(20, 30);
				 
				 float r = atan2f(dudePos.x - enemyPos[j].x, dudePos.z - enemyPos[j].z);
				 enemySpeed[j].x = sinf(r) * enemySpeedMax;
				 enemySpeed[j].z = cosf(r) * enemySpeedMax;
				 break;
			 }
		 }
	// }
}

void renderEnemies(){
	for(int i = 0; i < enemyC; i ++){
		sceGumPushMatrix();
		
		sceGumTranslate(&enemyPos[i]);
		sceGumRotateY(atan2f(dudePos.x - enemyPos[i].x, dudePos.z - enemyPos[i].z));
		sceGumScale(&enemyScale);
		enemy->render();
		
		sceGumPopMatrix();
	}
}


gwTexture *tex, *floorTex, *bushTex, *shadowTex, *bulletTex, *bulletfTex;

ScePspFVector3 eye = {0,7,10};
ScePspFVector3 eye2= {0,11.5,0.01};
ScePspFVector3 up = {0,1,0};
ScePspFVector3 center = {0,0,0};

float rot = 0;

gwVertTNV floorVerts[4] = {{0,0,0,1,0,-0.5,0,-0.5},{0,1,0,1,0,-0.5,0,0.5},{1,1,0,1,0,0.5,0,0.5},{1,0,0,1,0,0.5,0,-0.5}};
ScePspFVector3 floorScale = {4.3,1,4.3};
ScePspFVector3 floorPos = {0,-1,0};
int floorRad = 7;

ScePspFVector3 shadowPos = {0,-0.99f,0};
ScePspFVector3 shadowScale = {0,-1.0,0};

gwVertTNV bushVerts[4] = {{0,0,0,1,0,-0.5,0.5,0},{0,1,0,1,0,-0.5,-0.5,0},{1,1,0,1,0,0.5,-0.5,0},{1,0,0,1,0,0.5,0.5,0}};
ScePspFVector3 bushScale = {2,2,1};
ScePspFVector3 bushPos[14*14];
float bushRotX, bushRotY;


ScePspFVector3 bulletScale = {0.5,1,0.5};
ScePspFVector3 bulletfScale = {2,1,4};
ScePspFVector3 buletfTrans = {0,-0.98f,0};
ScePspFVector3 flashPos = { 2,0,2 };
const int bulletCount = 25;
ScePspFVector3 bulletPos[bulletCount];
ScePspFVector3 bulletVel[bulletCount];
float bulletRotY[bulletCount];
float bulletSpeed = 0.5f;
float lastFired = 0;
float rateOfFire = 0.15f;


void renderBullets(){
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_FIX, 0, 0xFFFFFFFF);
	
	for(int i = 0; i < bulletCount; i ++){
		if(!bulletVel[i].x == 0 && !bulletVel[i].z == 0){
			sceGuDepthMask(GU_TRUE);
			sceGumPushMatrix();
			bulletTex->activate();
			sceGuTexScale(1.0f, 1.0f);
			sceGumTranslate(&bulletPos[i]);
			sceGumRotateY(bulletRotY[i]);
			sceGumScale(&bulletScale);
			sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, floorVerts);
			
			bulletfTex->activate();
			sceGuTexScale(1.0f, 1.0f);
			sceGumTranslate(&buletfTrans);
			sceGumScale(&bulletfScale);
			sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, floorVerts);
			
			sceGumPopMatrix();
			sceGuDepthMask(GU_FALSE);
		}
	}
		sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);

}


void renderBushes(){
	int rad = 7;
	
	shadowTex->activate();
	sceGuTexScale(1.0f, 1.0f);
	
	shadowScale.x = bushScale.x;
	shadowScale.z = bushScale.y * 2.0f;
	shadowPos.y = -0.99f;

	
	for(int z = -rad, i = 0; z < rad; z ++)
		for(int x = -rad; x < rad; x ++){
			
			sceGuDepthMask(GU_TRUE);
			shadowPos.x = bushPos[i].x - 0.8f;
			shadowPos.z = bushPos[i].z - shadowScale.z * 0.2f;
			sceGumPushMatrix();
			sceGumTranslate(&shadowPos);
			sceGumRotateY(0.5f);
			sceGumScale(&shadowScale);
			sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, floorVerts);
			sceGumPopMatrix();
			sceGuDepthMask(GU_FALSE);
			i++;
		}
	
	bushTex->activate();
	sceGuTexScale(1.0f, 1.0f);
	
	for(int z = -rad, i = 0; z < rad; z ++)
		for(int x = -rad; x < rad; x ++){	
			sceGumPushMatrix();
			sceGumTranslate(&bushPos[i]);
			//bushRotY = atan2f(eye.x - bushPos[i].x, eye.z - bushPos[i].z);
		//	bushRotX = atan2f(eye.z - bushPos[i].z, eye.y - bushPos[i].y) - 1.0;
			sceGumRotateX(bushRotX);
			sceGumRotateY(bushRotY);
			sceGumScale(&bushScale);
			sceGuColor(0);
			sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, bushVerts);
			sceGumPopMatrix();
			i++;
		}
	
	
	
}

void floorRender(){
	floorTex->activate();
	sceGuTexScale(1.0f, 1.0f);
	
	for(int z = -floorRad; z < floorRad; z ++)
		for(int x = -floorRad; x < floorRad; x ++){
			floorPos.x = x * floorScale.x;
			floorPos.z = z * floorScale.z;
			sceGumPushMatrix();
			sceGumTranslate(&floorPos);
			sceGumScale(&floorScale);
			sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, floorVerts);
			sceGumPopMatrix();
		}
}

void renderDude(){
	
	sceGuDepthMask(GU_TRUE);
	shadowScale.x = bushScale.x;
	shadowScale.z = bushScale.y * 2.0f;
	shadowPos.x = dudePos.x - 0.6f;
	shadowPos.z = dudePos.z - shadowScale.z * 0.25f;
	shadowPos.y = -0.99f;
	sceGumPushMatrix();
	shadowTex->activate();
	sceGuTexScale(1.0f, 1.0f);
	sceGumTranslate(&shadowPos);
	sceGumRotateY(0.5f);
	sceGumScale(&shadowScale);
	sceGumDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, 4, 0, floorVerts);
	sceGumPopMatrix();
	sceGuDepthMask(GU_FALSE);
	
	sceGumPushMatrix();
	sceGumTranslate(&dudePos);
	sceGumRotateXYZ(&dudeRot);
	sceGumScale(&dudeScale);
	dude->render();
	
	sceGumTranslate(&gunPos);
	sceGumRotateXYZ(&gunRot);
	sceGumScale(&gunScale);
	gun->render();
	
	sceGumPopMatrix();
}

float absf(float a){
	if(a > 0) return a;
	else return -a;
}

char collided(ScePspFVector3 *pos1, float scale1, ScePspFVector3 *pos2, float scale2){
	if(pos1->x + scale1 > pos2->x - scale2)//right of 1 with left of 2
		if(pos1->x - scale1 < pos2->x + scale2)//left of 1 with right of 2
			if(pos1->y + scale1 > pos2->y - scale2)//top of 1 with bottom of 2
				if(pos1->y - scale1 < pos2->y + scale2)//botom of 1 with top of 2
					if(pos1->z + scale1 > pos2->z - scale2)//back of 1 with front of 2
						if(pos1->z - scale1 < pos2->z + scale2)//front of 1 with back of 2
							return 1;
	return 0;
}	


ScePspFVector3 lastPos;
gwTimer_callback update(void *pData){
	//return NULL;
	
	//gwRender *render = reinterpret_cast<gwRender*>(pData);
	gwTimer *timer = reinterpret_cast<gwTimer*>(pData);
	if(health > 0){
	
		if(analog.x !=0 || analog.y != 0)
		dudeRot.y = rot = atan2f(analog.x, analog.y);
		
		if(pad.Buttons & PSP_CTRL_CROSS)
			dudeSpeed.z = dudeSpeedMax.z;
		else if(pad.Buttons & PSP_CTRL_TRIANGLE)
			dudeSpeed.z  = -dudeSpeedMax.z;
		else dudeSpeed.z = 0;

		if(pad.Buttons & PSP_CTRL_SQUARE)
			dudeSpeed.x = -dudeSpeedMax.x;
		else if(pad.Buttons & PSP_CTRL_CIRCLE)
			dudeSpeed.x  = dudeSpeedMax.x;
		else dudeSpeed.x = 0;

		if(absf(dudeSpeed.x) == absf(dudeSpeed.z)){
			dudeSpeed.x *= 0.6f;
			dudeSpeed.z *= 0.6f;
		}
		
		if(!(pad.Buttons & PSP_CTRL_CROSS)&&!(pad.Buttons & PSP_CTRL_TRIANGLE)&&!(pad.Buttons & PSP_CTRL_SQUARE)&&!(pad.Buttons & PSP_CTRL_CIRCLE)){
			dudeSpeed.x = sinf(rot) * dudeSpeedMax.x * absf(analog.x);
			dudeSpeed.z = cosf(rot) * dudeSpeedMax.z  * absf(analog.y);
		}
		
		if(dudePos. x < (floorRad-2) * -floorScale.x){
			dudeSpeed.x = 0;
			dudePos.x = (floorRad-2) * -floorScale.x;
		}
		if(dudePos. x > (floorRad-3) * floorScale.x){
			dudeSpeed.x = 0;
			dudePos.x = (floorRad-3) * floorScale.x;
		}
		if(dudePos. z < (floorRad-2) * -floorScale.z){
			dudeSpeed.z = 0;
			dudePos.z = ((floorRad-2) * -floorScale.z);
		}
		if(dudePos. z > (floorRad-2) * floorScale.z){
			dudeSpeed.z = 0;
			dudePos.z = ((floorRad-2) * floorScale.z);
		}
		
		dudePos.x += dudeSpeed.x;
		dudePos.z += dudeSpeed.z;
		
		//dudeRot.y = atan2f(dudePos.x - lastPos.x, dudePos.z - lastPos.z);

		//fire bullet
		if(pad.Buttons & PSP_CTRL_LTRIGGER || pad.Buttons & PSP_CTRL_RTRIGGER){
			if(timer->elapsedTime() - lastFired >= rateOfFire){
				for(int i = 0; i < bulletCount; i ++){//find unused bullet
					if(bulletVel[i].x == 0 && bulletVel[i].z == 0){//unused
						flashPos.x = dudePos.x + sinf(rot) * 1.25f;
						flashPos.z = dudePos.z + cosf(rot) * 1.25f;
						bulletPos[i].x = dudePos.x + cosf(rot) * -0.2f;
						bulletPos[i].z = dudePos.z + sinf(rot) * 0.2;
						bulletRotY[i] = dudeRot.y;
						bulletVel[i].x = sinf(rot) * bulletSpeed;
						bulletVel[i].z = cosf(rot) * bulletSpeed;
						lastFired = timer->elapsedTime();
						break;
					}
				}
			}
		}
	}
	
	if(timer->elapsedTime() - lastFired >= rateOfFire*0.5f)
		flashPos.x = 100;
	
	if(timer->elapsedTime() - lastWave >= waveFreq){
		//release baddies
	//	printf("\n\x1b[33mRelease Baddies\n");
	//	releaseEnemy();
		lastWave = timer->elapsedTime();
	}
	
	//update bullets
	
	for(int i = 0; i < bulletCount; i ++){
		if(absf(bulletPos[i].x - dudePos.x) > 10 || absf(bulletPos[i].z - dudePos.z) > 10)
			bulletVel[i].x = bulletVel[i].z = 0;
		
		for(int j = 0; j < enemyMax; j++){
			if(collided(&bulletPos[i], 0.25, &enemyPos[j], 0.25)){
				bulletPos[i].x = 100;
			   bulletVel[i].x = bulletVel[i].z = 0;
			   enemySpeed[j].x = enemySpeed[j].z = 0;
				enemyPos[j].x = -100;
				kills ++;
				
				releaseEnemy();
				break;
				
			}
		}
		
		//if(!bulletVel[i].x == 0 && !bulletVel[i].z == 0){
			bulletPos[i].x += bulletVel[i].x;
			bulletPos[i].z += bulletVel[i].z;
		//}
	}
	
	
	for(int i = 0; i < enemyC; i++){
		enemyPos[i].x += enemySpeed[i].x;
		enemyPos[i].z += enemySpeed[i].z;
		
		float rrot = atan2f(dudePos.x - enemyPos[i].x, dudePos.z - enemyPos[i].z);
		
		enemySpeed[i].x = sinf(rrot) * enemySpeedMax;
		enemySpeed[i].z = cosf(rrot) * enemySpeedMax;
		
		if(collided(&dudePos, 0.7, &enemyPos[i], 0.25)){
			health --;
			enemySpeed[i].x = enemySpeed[i].z = 0;
			enemyPos[i].x = -100;
		}
	}
	
	if(health <= 0){
		dudeRot.x = 1.57;
		
		if(pad.Buttons & PSP_CTRL_START){
			health = 5;
			kills = 0;
			for(int i = 0; i < enemyMax; i ++){
				enemySpeed[i].x = enemySpeed[i].z = 0;
			}
			
			enemyC = 10;
			
			dudePos.x = dudePos.z = 0;
			
			for(int i = 0; i < enemyC; i++){
				releaseEnemy();		
			}
			
			dudeRot.x = 0;
			
			
		}
	}
	
	if(enemyC < 40 && kills == 1000){
		enemyC += 2;
		releaseEnemy();
		releaseEnemy();
	}
	if(enemyC < 40 && kills == 2000){
		enemyC += 2;
		releaseEnemy();
		releaseEnemy();
	}
	if(enemyC < 40 && kills == 3000){
		enemyC += 2;
		releaseEnemy();
		releaseEnemy();
	}if(enemyC < 40 && kills == 4000){
		enemyC += 2;
		releaseEnemy();
		releaseEnemy();
	}
	
	return NULL;
}

gwTimer_callback enemyRelease(void *d){
	//gwTimer *timer = reinterpret_cast<gwTimer*>(d);
//	srand(timer->elapsedTime());
	//for(int i = 0; i < random(1,5); i ++)
		//releaseEnemy();
	//	printf("\n\x1b[33mRelease Baddies\n");
	return NULL;
}

gwRender *render;
gwTimerEvent *splashE;

gwTexture *splash;

gwTimer_callback splashU(void *d){
	gwTimer *timer = reinterpret_cast<gwTimer*>(d);
	if(timer->elapsedTime() >= 6.28){
		timer->deleteEvent(splashE);
		splashE = NULL;
		timer->newEvent(1.f/60.f, (gwTimer_callback) update, timer);
	}
	if(splashE){
	
	char a = 255.f - 255.f * (sinf(timer->elapsedTime() * 0.5f));
	
	render->start();
	render->clearScreen();
	render->texture(splash, &fullScreen);
	render->rectangle(&fullScreen,(a << 24)|0x00000000);
	render->end();
	}
}

int main(int argc, void *argv[]){
	GearWorks::init();
	pspDebugScreenInit();
	pspDebugScreenEnableBackColor(0);
	render = new gwRender(GW_PIXEL_FORMAT_8888);
	render->setClearColor(0);
	render->setPrimitiveRenderMode(GW_FILL);
	render->start();
	
	sceGuEnable(GU_FOG);
	sceGuFog(8, 25, 0);
	
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
	sceGuLightColor(0,GU_DIFFUSE,0xFFAAAAAA);
	sceGuLightColor(0,GU_SPECULAR,0xFFFFFFFF);
	sceGuLightAtt(0,0.f,2.f,0.f);
	
	sceGuEnable(GU_LIGHT1);
	
	sceGuLight(1,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&flashPos);
	sceGuLightColor(1,GU_DIFFUSE,0xFF0000FF);
	sceGuLightColor(1,GU_SPECULAR,0xFF00FFFF);
	sceGuLightAtt(1,0.f,2.f,0.f);
	
	

	sceGuSpecular(12.0f);
	sceGuAmbient(0xff999999);
	render->end();

	
	gwTimer *timer = new gwTimer();
	splashE = timer->newEvent(1.f/60.f, (gwTimer_callback) splashU, timer);
	splash = new gwTexture("resources/splash.png");
	timer->newEvent(1.f, (gwTimer_callback) enemyRelease, timer);
	
	gwTexture *screen = new gwTexture(480, 272, GW_PIXEL_FORMAT_5650, GW_VRAM);
	//gwRect halfScreen = {240-64, 136-64,128,128};
	
	pgeFont *font = pgeFontLoad("resources/font.ttf", 12, PGE_FONT_SIZE_POINTS, 128);
	//intraFont *ltn = intraFontLoad("flash0:/font/ltn1.pgf",0);
	
	floorTex = new gwTexture("resources/grass.png");
	floorTex->swizzle();
	floorTex->toVram();
	
	bushTex = new gwTexture("resources/bigbush.png");
	bushTex->swizzle();
	bushTex->toVram();
	bushRotY = atan2f(eye.x, eye.z);
	bushRotX = atan2f(eye.z, eye.y) - 1.0;

	srand(rand() %100);
	
	for(int z = -floorRad, i = 0; z < floorRad; z ++)
		for(int x = -floorRad; x < floorRad; x ++){
			
			bushPos[i].x = x * floorScale.x + random(-floorScale.x,floorScale.x);
			bushPos[i].z = z * floorScale.z + random(-floorScale.z,floorScale.z);
			bushPos[i].y = 0;
			
			i++;
		}
	
	shadowTex = new gwTexture("resources/shadow.png");
	shadowTex->swizzle();
	shadowTex->toVram();
	
	bulletTex = new gwTexture("resources/bullet.png");
	bulletTex->swizzle();
	bulletfTex = new gwTexture("resources/bulletf.png");
	bulletfTex->swizzle();
	
	for(int i = 0; i < bulletCount; i ++){
		bulletVel[i].x = 0;
		bulletVel[i].z = 0;
		bulletPos[i].y = 0.1f;
		bulletPos[i].x = -100;
	}
	
	dude = new gwObj("resources/H3gun.obj");
	tex = new gwTexture("resources/H3.png");
	tex->swizzle();
	dude->bindTexture(tex);
	
	enemy = new gwObj("resources/smallenemy.obj");
	tex = new gwTexture("resources/smallenemy.png");
	tex->swizzle();
	enemy->bindTexture(tex);
	
	for(int i = 0; i < enemyC; i++){
		releaseEnemy();		
	}
	
	gun = new gwObj("resources/gun.obj");
	tex = new gwTexture("resources/gun.png");
	tex->swizzle();
	gun->bindTexture(tex);
	
	gwObj *cool = new gwObj("resources/cool.obj");
cool->bindTexture(screen);
	cool->setTexScale(480.f/512.f, 272.f/512.f);
		
	sceCtrlSetSamplingCycle(0);
	sceCtrlSetSamplingMode(PSP_CTRL_MODE_ANALOG);
		
	printf("\x1b[2J");//clear terminal
	
	while(splashE != NULL)timer->update();//idle for splash screen
		
	while (GearWorks::isRunning()) {
		printf("\x1b[H");
		sceCtrlPeekBufferPositive(&pad, 1); 
		
		if (pad.Lx > 103 && pad.Lx < 151)	analog.x	= 0.0f;
		else												analog.x	= (pad.Lx - 128.0f) / 128.0f;
		
		if (pad.Ly > 103 && pad.Ly < 151)	analog.y	= 0.0f;
		else												analog.y	= (pad.Ly - 128.0f) / 128.0f;
		
		timer->update();
		
		render->start();
		render->clearScreen();
	//	intraFontSetStyle(ltn, 1.0f,0xFFFFFFFF,0xffffffff,INTRAFONT_ALIGN_CENTER);
		//intraFontPrintf(ltn, 20, 20, "Killz:%03d",kills);
		
		//sceGuClearDepth(0);
		//sceGuClear(GU_DEPTH_BUFFER_BIT);
		//render->rectangle(&fullScreen, 0x44000000);
						
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();
		eye.x = dudePos.x;
		eye.z = dudePos.z + 7.f;
		sceGumLookAt(&eye, &dudePos, &up);
		
		pos.x = dudePos.x + 2;
		pos.z = dudePos.z + 2;
		sceGuLight(0,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&pos);
		sceGuLight(1,GU_POINTLIGHT,GU_DIFFUSE_AND_SPECULAR,&flashPos);

		
		sceGumMatrixMode(GU_MODEL);
				
		//render->toTexture(screen);
		//render->setClearColor(0xff00ff);
		//render->clearScreen();
		//screen->setAllPixels(0xff000000);
		floorRender();

		renderBushes();
		
		renderEnemies();
		
		renderDude();

		renderBullets();

		pgeFontActivate(font);
		pgeFontPrintf(font, 100, 50, 0xff0000aa, "Killz:%03d", kills);

		//render->texture(tex,&fullScreen,0,(127 << 24)|0x00000000);
		
	/*	render->toScreen();
		//render->setClearColor(0);
		render->clearScreen();
		//sceGuDisable(GU_FOG);
		//sceGuBlendFunc(GU_ADD, GU_ONE_MINUS_SRC_COLOR, GU_FIX, 0, 0xFFFFFFFF);
		//render->texture(screen, &fullScreen, 0 , 100);
		//sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
		sceGumMatrixMode(GU_VIEW);
		sceGumLoadIdentity();

		sceGumLookAt(&eye2, &center, &up);
		
		sceGumMatrixMode(GU_MODEL);
		sceGuDisable(GU_LIGHTING);
		cool->render();
		sceGuEnable(GU_LIGHTING);
		*/
		
		pspDebugScreenSetXY(0,0);
		pspDebugScreenPrintf("Killz:%03d\t Health:%d", kills, health);
		render->end();
		printf("\x1b[33mFPS:\x1b[36m%f\n", timer->fps());
		//pspDebugScreenSetOffset((int)render->getFrameBuffer());
	}
	
	
	delete timer;
	GearWorks::exitGame();
	return 1;
}
