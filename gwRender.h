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
	//void *doneBuffer;//for triple buffering
	void *dispBuffer;
	void *depthBuffer;
	
	gwTexture *disp;
	gwTexture *back;
	gwTexture *done;
	gwTexture *depth;
	
	bool renderToTarget;
	gwTextureS *rTarget;
	gwTexture *last;//last rendered tex, to avoid double activation
	//extern xPspGeContext *gwGeContext;
	
	int guStates;
	
	float lastRot;
	gwMatrix4 rotMatrix;
	
	static void tripleBufferCallback(void **display, void **render);
	void updateDoneBuffer();
	void renderFullscreenTexture( float u0, float v0, float u1, float v1 );
	
public:
	
	gwRender();
	gwRender(enum gwPixelFormat format);
	~gwRender();
	
	//frame info's
	int getFrameBuffOffset();
	gwTexture *getFrameBuff();
	gwTexture *getDispBuff();
	gwTexture *getBackBuff();
	gwTexture *getDepthBuff();
	
	
	void setClearColor(unsigned int col);
	void clearScreen();
	
	bool toTexture(gwTexture *target, gwRect *viewPort = (gwRect*)((void*)0));//render target
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
	void textureSpriteUv(gwTexture *tex, gwRect *destRect, gwRect *srcRect, float angle = 0.0f, unsigned char alpha = 255);


	//gwRender::primitive2D
	void setPrimitiveRenderMode(enum gwPrimitiveRenderMode mode);
	void rectangle(gwRect *rect, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void gradient(gwRect *rect, unsigned int col1 = 0xFFFFFFFF , unsigned int col2 = 0xFFFF0000, unsigned int col3 = 0xFF00FF00, unsigned int col4 = 0xFF0000FF);
	void ellipse(gwRect *destRect, char steps = 32,unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void ellipseGradient(gwRect *destRect, char steps = 32,unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void star(gwRect *destRect, float innerRadius = 0,int points = 5, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void gear(gwRect *destRect, float innerRadius = 0,int points = 5, unsigned int color = 0xFFFFFFFF, float angle = 0.0f);
	void line(float x1, float y1, float x2, float y2, unsigned int color = 0xFFFFFFFF);
	void point(float x, float y, unsigned int color = 0xFFFFFFFF);
	void polygon(gwVertV *vertices, int vertCount, unsigned int color = 0xFFFFFFFF);

	//gwRender::lowLevel
	void enable(enum gwGuState state);
	void disable(enum gwGuState state);
	unsigned int states();
	void states(int states);
	
	//gwRender::3D
	//using built in matrices
	void matrixMode(enum gwMatrixMode mode);
	void identityMatrix();
	void perspectiveMatrix(float fov = 45.f, float aspect = 1.765f, float near = 1.f, float far = 100.f);
	void orthographicMatrix(float left = 0.f, float right = 480.f, float bottom = 272.f, float top = 0.f, float near = 0.f, float far = 100.f);
	void lookatMatrix(gwVector3 &pos, gwVector3 &center, gwVector3 &up);
	
	void pushMatrix();
	void popMatrix();
	void rotate(float rot, float x, float y, float z);
	void rotate(float rot, gwVector3 &axis);
	void rotate(gwVector3 &rot);
	void translate(float x, float y, float z);
	void translate(gwVector3 &trans);
	void scale(float x, float y, float z);
	void scale(gwVector3 &scale);
	
	//external matrices
	void loadMatrix(gwMatrix4 &matrix);
	void multMatrix(gwMatrix4 &matrix);
	void storeMatrix(gwMatrix4 &matrix);
	void identityMatrix(gwMatrix4 &matrix);
	void inverseMatrix();
	void inverseOrthoMatrix();
	
	//gwRender::effects
	void convolution5x5(gwTexture *src, gwTexture *dst);
	void convolution9x9(gwTexture *src, gwTexture *dst);

	
	
	
};

#endif
