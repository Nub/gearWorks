/*************************************************************************\
 Engine: gearWorks
 File: gwRender.cpp
 Author: Zachry Thayer
 Description: Loads png images and saves them requires -lpng -lz
\*************************************************************************/
#ifndef _GW_RENDER_H
#define _GW_RENDER_H

#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 272
#define BUFFER_WIDTH 512
#define FRAME_BUFFER_SIZE (BUFFER_WIDTH*SCREEN_HEIGHT)

#include "gwTypes.h"
#include "gwTexture.h"


class gwRender{
private:
	bool initialized;
	bool isRendering;//for checking if inside render context
	int currentDispList;
	unsigned int clearColor;
	enum gwPrimitiveRenderMode primitiveRenderMode;

	int bpp;
	enum gwPixelFormat frameFormat;
	void *frameBuffer;
	void *backBuffer;
	void *dispBuffer;
	void *depthBuffer;
	
	bool renderToTarget;
	gwTextureS *rTarget;
			
	//extern xPspGeContext *gwGeContext;
	
public:
	
	gwRender();
	gwRender(enum gwPixelFormat format);
	~gwRender();
	
	unsigned int *getFrameBuffer();
	
	void setClearColor(unsigned int col);
	void clearScreen();
	
	bool toTexture(gwTexture *target);//render target
	void toScreen();//render framebuffer
	
	void start();//open render context
	void end(bool waitVSync = false);//close render context
	
	//only to be called inside a rendering context
	void *getVertexMem(int size);
	gwRect *tmpRect(float x, float y, float w, float h);
	gwPoint *tmpPoint(float x,float y, float z);
	
	//gwRender::gwTexture
	//simple render scaling
	void texture(gwTexture *tex, gwRect *destRect, float angle = 0.0f, unsigned char alpha = 255);
	void textureSprite(gwTexture *tex, gwRect *destRect, gwRect *srcRect, float angle = 0.0f, unsigned char alpha = 255);

	//gwRender::primitive 2D
	void setPrimitiveRenderMode(enum gwPrimitiveRenderMode mode);
	void rectangle(gwRect *rect, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void gradient(gwRect *rect, unsigned int col1 = 0xFFFFFFFF , unsigned int col2 = 0xFFFF0000, unsigned int col3 = 0xFF00FF00, unsigned int col4 = 0xFF0000FF);
	void ellipse(gwRect *destRect, char steps = 32,unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void star(gwRect *destRect, float innerRadius = 0,int points = 5, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void gear(gwRect *destRect, float innerRadius = 0,int points = 5, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void line(float x1, float y1, float x2, float y2, unsigned int color = 0xFFFFFFFF);
	void point(float x, float y, unsigned int color = 0xFFFFFFFF);
	void polygon(gwVertV *vertices, int vertCount, unsigned int color = 0xFFFFFFFF);

	//gwRender:: 3D
	//void setCamera(gwCamera *camera);
	//void setProjection(float fov = 70.f);
	
};

#endif
