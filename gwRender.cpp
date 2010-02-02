/*************************************************************************\
 Engine: gearWorks
 File: gwRender.cpp
 Author: Zachry Thayer
 Description: Handles the Gu Settings and Rendering Items 
 Requires: -lpspgu -lpspgum -lpspvfpu
\*************************************************************************/
#include "gwRender.h"
#include "gwVram.h"

#include <pspkernel.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspdisplay.h>
#include <math.h>

static unsigned int __attribute__((aligned(16))) gwDisplayList[2][262144/4];
static unsigned int __attribute__((aligned(16))) gwCallList[64];


static const ScePspIMatrix4 gwGfxDitherMatrix =	{{0,  8,  2, 10},
												{12,  4, 14,  6},
												{3,  11,  1,  9},
												{15,  7, 13,  5}};

PspGeContext __gwGeContext;
void *__gwdoneBuffer;//for triple buffering


gwRender::gwRender(){
	gwRender(GW_PIXEL_FORMAT_8888);
}

gwRender::gwRender(enum gwPixelFormat format){
	initialized = true;
	isRendering = false;
	currentDispList = 1;
	renderToTarget = false;
	primitiveRenderMode = GW_FILL;
	
	switch(format)
	{
		case GW_PIXEL_FORMAT_4444:
		case GW_PIXEL_FORMAT_5650:
		case GW_PIXEL_FORMAT_5551:
			bpp = 2;
			frameFormat = format;
			break;
		default:
			bpp = 4;
			frameFormat = GW_PIXEL_FORMAT_8888;
			break;
	}
	
	back = new gwTexture(SCREEN_WIDTH,SCREEN_HEIGHT, format, GW_VRAM);
	disp = new gwTexture(SCREEN_WIDTH,SCREEN_HEIGHT, format, GW_VRAM);
	depth = new gwTexture(SCREEN_WIDTH,SCREEN_HEIGHT, GW_PIXEL_FORMAT_4444, GW_VRAM);
	
	backBuffer = gwVramRelativePointer(back->getTextureS()->data);
	dispBuffer = gwVramRelativePointer(disp->getTextureS()->data);
	depthBuffer = gwVramRelativePointer(depth->getTextureS()->data);
	
	sceGuInit();
	
	// setup GU
	start();
	sceGuDrawBuffer(frameFormat, backBuffer, BUFFER_WIDTH);
	sceGuDispBuffer(SCREEN_WIDTH, SCREEN_HEIGHT, dispBuffer, BUFFER_WIDTH);
	sceGuDepthBuffer(depthBuffer, BUFFER_WIDTH);
	
	sceGuOffset(2048 - (SCREEN_WIDTH>>1), 2048 - (SCREEN_HEIGHT>>1));
	sceGuViewport(2048, 2048, SCREEN_WIDTH, SCREEN_HEIGHT);
	
	// Scissoring
	sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	enable(GW_SCISSOR_TEST);
	
	// Backface culling
	sceGuFrontFace(GU_CCW);
	enable(GW_CULL_FACE);
	
	// Depth test
	sceGuDepthRange(65535, 0);
	sceGuDepthFunc(GU_GEQUAL);
	enable(GW_DEPTH_TEST);
	//sceGuDepthMask(GU_TRUE);		// disable z-writes
	
	enable(GW_CLIP_PLANES);
	
	// Texturing
	enable(GW_TEXTURE_2D);
	sceGuTexWrap(GU_REPEAT, GU_REPEAT);
	sceGuTexFilter(GU_LINEAR,GU_LINEAR);
	sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
	sceGuTexEnvColor(0xFFFFFFFF);
	sceGuColor(0xFFFFFFFF);
	sceGuAmbientColor(0xFFFFFFFF);
	sceGuTexOffset(0.0f, 0.0f);
	sceGuTexScale(1.0f, 1.0f);
	
	// Blending
	enable(GW_BLEND);
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	
	sceGuSetDither(&gwGfxDitherMatrix);

	if(bpp < 4)
	{
		enable(GW_DITHER);
	}
	
	sceGuAlphaFunc(GU_GREATER,0,0xff);
	enable(GW_ALPHA_TEST);
	
	sceGuShadeModel(GU_SMOOTH);

	
	// Projection
	matrixMode(GW_PROJECTION);
	identityMatrix();
	perspectiveMatrix();
	
	matrixMode(GW_VIEW);
	identityMatrix();
	
	matrixMode(GW_MODEL);
	identityMatrix();
	
	clearColor = 0xFF000000;
	sceGuClearColor(clearColor);
	sceGuClear(GU_COLOR_BUFFER_BIT);
	
	//sceGuFinish();
	//sceGuSync(0,0);
	
//	sceDisplayWaitVblankStart();
	//frameBuffer = gwVramAbsolutePointer(sceGuSwapBuffers());
	end();
	sceGuDisplay(1);
	
}

gwRender::~gwRender(){
	initialized = false;
	//gwVramFree(dispBuffer);
	//gwVramFree(backBuffer);
	//gwVramFree(depthBuffer);
	if(done)
		delete done;
	delete disp;
	delete back;
	delete depth;
}

void gwRender::tripleBufferCallback(void **display, void **render){
	void* active = __gwdoneBuffer;
	__gwdoneBuffer = *display;
	*display = active;
}

void gwRender::updateDoneBuffer(){
	if(done)
		done->getTextureS()->data = __gwdoneBuffer;
}

int gwRender::getFrameBuffOffset(){
	if(!initialized) return 0;
	return (int)gwVramRelativePointer(frameBuffer);
}

gwTexture *gwRender::getFrameBuff(){
	if(!initialized) return NULL;
	disp->getTextureS()->data = frameBuffer;
	return disp;
}

gwTexture *gwRender::getDispBuff(){
	if(!initialized) return NULL;
	return disp;
}
gwTexture *gwRender::getBackBuff(){
	if(!initialized) return NULL;
	return back;
}
gwTexture *gwRender::getDepthBuff(){
	if(!initialized) return NULL;
	return depth;
}

void gwRender::setClearColor(unsigned int col){
	if(initialized){
		clearColor = col;
	}
}

void gwRender::clearScreen(){
	if(isRendering && initialized){
		sceGuClearColor(clearColor);
		sceGuClearDepth(0);
		sceGuClear(GU_COLOR_BUFFER_BIT|GU_DEPTH_BUFFER_BIT|GU_FAST_CLEAR_BIT);
	}
}


bool gwRender::toTexture(gwTexture *target, gwRect *viewPort){
	if(initialized){
		rTarget = target->getTextureS();
		if(rTarget->location != GW_VRAM){
			return false;
		}
		else{
			if(isRendering){
				sceGuSync(0,2);
				
				sceKernelDcacheWritebackAll();
				
				sceGuDrawBufferList(rTarget->format, gwVramRelativePointer(rTarget->data), rTarget->textureWidth);
				
				if(viewPort == NULL){
					sceGuOffset(2048 - (rTarget->width>>1), 2048 - (rTarget->height>>1));
					sceGuViewport(2048, 2048, rTarget->width, rTarget->height);
					sceGuScissor(0, 0, rTarget->width, rTarget->height);
				}else{
					float ws = (rTarget->width/viewPort->w);
					float hs = (rTarget->height/viewPort->h);
					int w = (int)ws*rTarget->width;
					int h = (int)hs*rTarget->height;
					sceGuOffset(2048 - (w>>1), 2048 - (h>>1));
					sceGuViewport((int)(2048 - viewPort->x*ws),(int)(2048 - viewPort->y*hs), w,h);
					sceGuScissor(0, 0, rTarget->width, rTarget->height);
				}
				return true;
			}
		}
	}
	return false;
}

void gwRender::toScreen(){
	if(initialized){
		if(isRendering){
			sceGuSync(0,2);
			
			sceKernelDcacheWritebackAll();
			
			sceGuDrawBufferList(frameFormat, gwVramRelativePointer(frameBuffer), BUFFER_WIDTH);
			
			sceGuOffset(2048 - (SCREEN_WIDTH>>1), 2048 - (SCREEN_HEIGHT>>1));
			sceGuViewport(2048, 2048, SCREEN_WIDTH,SCREEN_HEIGHT);
			sceGuScissor(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
		}
	}
}

void gwRender::start(){
	if(!isRendering && initialized){
		isRendering = true;
		
	/*	sceGuStart(GU_DIRECT, gwCallList);
			sceGuCallList(gwDisplayList[currentDispList]);
		sceGuFinish();
		
		currentDispList ^= 1;//swaparoo
		frameBuffer = gwVramAbsolutePointer(sceGuSwapBuffers());
*/
		
		sceGuStart(GU_DIRECT, gwDisplayList);//start next list
				
	}
}

void gwRender::end(bool waitVSync){
	if(isRendering && initialized){
		isRendering = false;
		
		sceGuFinish();
		sceGuSync(0,0);
		
		if(waitVSync)
			sceDisplayWaitVblankStart();
				
		frameBuffer = gwVramAbsolutePointer(sceGuSwapBuffers());
		
		if(guStates & (1<<GW_TRIPLE_BUFFER))
			updateDoneBuffer();
	}
}

void *gwRender::getVertexMem(int size){
	if(isRendering && initialized){
		return sceGuGetMemory(size);
	}
	else return NULL;
}

gwRect *gwRender::tmpRect(float x, float y, float w, float h){
	if(isRendering && initialized){
		gwRect *ret = (gwRect*)sceGuGetMemory(sizeof(gwRect));
		if(!ret) return NULL;
		ret->x = x;
		ret->y = y;
		ret->w = w;
		ret->h = h;
		return ret;
	}
	else return NULL;
}

gwPoint *gwRender::tmpPoint(float x,float y, float z){
	if(isRendering && initialized){
		gwPoint *ret = (gwPoint*)sceGuGetMemory(sizeof(gwPoint));
		if(!ret) return NULL;
		ret->x = x;
		ret->y = y;
		ret->z = z;
		return ret;
	}
	else return NULL;
}

//gwTexture

void gwRender::texture(gwTexture *tex, gwRect *destRect, float angle, unsigned char alpha){
	if(isRendering && initialized){
		if(!tex) return;
		if(!destRect) return;
		
		sceGuDisable(GU_DEPTH_TEST);
		
		//if(tex != last){
			last = tex;
			last->activate();
	//	}
		gwTextureS *tmp = tex->getTextureS();
		
		if(alpha != 255)
		{
			sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
			sceGuColor(GU_RGBA(255, 255, 255, alpha));
		}else{
			sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
			sceGuColor(0xFFFFFFFF);
		}
		
		if(angle == 0.f){
			gwVertTV *vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<1);
			
			vertices[0].u = 0.f;
			vertices[0].v = 0.f;
			vertices[0].x = destRect->x;
			vertices[0].y = destRect->y; 
			vertices[0].z = 0;
			
			vertices[1].u = tmp->width;
			vertices[1].v = tmp->height;
			vertices[1].x = destRect->x + destRect->w;
			vertices[1].y = destRect->y + destRect->h;
			vertices[1].z = 0;
			
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
		}else{
			float x = destRect->x + destRect->w * 0.5f;
			float y = destRect->y + destRect->h * 0.5f;
			
			float c = cosf(angle), s = sinf(angle);
			
			float width = destRect->w * 0.5f;
			float height = destRect->h * 0.5f;
			
			float cw = c*width;
			float sw = s*width;
			float ch = c*height;
			float sh = s*height;
			
			gwVertTV* vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<2);
			
			vertices[0].u = 0.f;
			vertices[0].v = 0.f;
			vertices[0].x = x - cw + sh;
			vertices[0].y = y - sw - ch;
			vertices[0].z = 0;
			
			vertices[1].u = 0.f;
			vertices[1].v = tmp->height;
			vertices[1].x = x - cw - sh;
			vertices[1].y = y - sw + ch;
			vertices[1].z = 0;
			
			vertices[2].u = tmp->width;
			vertices[2].v = tmp->height;
			vertices[2].x = x + cw - sh;
			vertices[2].y = y + sw + ch;
			vertices[2].z = 0;
			
			vertices[3].u = tmp->width;
			vertices[3].v = 0.f;
			vertices[3].x = x + cw + sh;
			vertices[3].y = y + sw - ch;
			vertices[3].z = 0;
			
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		}
	}
	
	sceGuEnable(GU_DEPTH_TEST);
}

void gwRender::textureSprite(gwTexture *tex, gwRect *destRect, gwRect *srcRect, float angle, unsigned char alpha){
	if(isRendering && initialized){
		if(!tex) return;
		if(!destRect || !srcRect) return;
		
		tex->activate();
		//gwTextureS *tmp = tex->getTextureS();
		sceGuDisable(GU_DEPTH_TEST);
		sceGuDepthMask(GU_TRUE);

		if(alpha != 255)
		{
			sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
			sceGuColor(GU_RGBA(255, 255, 255, alpha));
		}else{
			sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
			sceGuColor(0xFFFFFFFF);
		}
		
		if(angle == 0.f){
			gwVertTV *vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<1);
			
			vertices[0].u = srcRect->x;
			vertices[0].v = srcRect->y;
			vertices[0].x = destRect->x;
			vertices[0].y = destRect->y; 
			vertices[0].z = 0;
			
			vertices[1].u = srcRect->x + srcRect->w;
			vertices[1].v = srcRect->y + srcRect->h;
			vertices[1].x = destRect->x + destRect->w;
			vertices[1].y = destRect->y + destRect->h;
			vertices[1].z = 0;
			
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
		}else{
			float x = destRect->x + destRect->w * 0.5f;
			float y = destRect->y + destRect->h * 0.5f;
			
			float c = cosf(angle), s = sinf(angle);
			
			float width = destRect->w * 0.5f;
			float height = destRect->h * 0.5f;
			
			float cw = c*width;
			float sw = s*width;
			float ch = c*height;
			float sh = s*height;
			
			gwVertTV* vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<2);
			
			vertices[0].u = srcRect->x;
			vertices[0].v = srcRect->y;
			vertices[0].x = x - cw + sh;
			vertices[0].y = y - sw - ch;
			vertices[0].z = 0;
			
			vertices[1].u = srcRect->x;
			vertices[1].v = srcRect->y + srcRect->h;
			vertices[1].x = x - cw - sh;
			vertices[1].y = y - sw + ch;
			vertices[1].z = 0;
			
			vertices[2].u = srcRect->x + srcRect->w;
			vertices[2].v = srcRect->y + srcRect->h;
			vertices[2].x = x + cw - sh;
			vertices[2].y = y + sw + ch;
			vertices[2].z = 0;
			
			vertices[3].u = srcRect->x + srcRect->w;
			vertices[3].v = srcRect->y;
			vertices[3].x = x + cw + sh;
			vertices[3].y = y + sw - ch;
			vertices[3].z = 0;
			
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		}
	}
	
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);

}


void gwRender::textureSpriteUv(gwTexture *tex, gwRect *destRect, gwRect *srcRect, float angle, unsigned char alpha){
	if(isRendering && initialized){
		if(!tex) return;
		if(!destRect || !srcRect) return;
		
		tex->activate();
		//gwTextureS *tmp = tex->getTextureS();
		
		if(alpha != 255)
		{
			sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
			sceGuColor(GU_RGBA(255, 255, 255, alpha));
		}else{
			sceGuTexFunc(GU_TFX_REPLACE, GU_TCC_RGBA);
			sceGuColor(0xFFFFFFFF);
		}
		
		if(angle == 0.f){
			gwVertTV *vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<1);
			
			vertices[0].u = srcRect->x;
			vertices[0].v = srcRect->y;
			vertices[0].x = destRect->x;
			vertices[0].y = destRect->y; 
			vertices[0].z = 0;
			
			vertices[1].u = srcRect->w;
			vertices[1].v = srcRect->h;
			vertices[1].x = destRect->x + destRect->w;
			vertices[1].y = destRect->y + destRect->h;
			vertices[1].z = 0;
			
			sceGuDrawArray(GU_SPRITES, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
		}else{
			float x = destRect->x + destRect->w * 0.5f;
			float y = destRect->y + destRect->h * 0.5f;
			
			float c = cosf(angle), s = sinf(angle);
			
			float width = destRect->w * 0.5f;
			float height = destRect->h * 0.5f;
			
			float cw = c*width;
			float sw = s*width;
			float ch = c*height;
			float sh = s*height;
			
			gwVertTV* vertices = (gwVertTV*)sceGuGetMemory(sizeof(gwVertTV)<<2);
			
			vertices[0].u = srcRect->x;
			vertices[0].v = srcRect->y;
			vertices[0].x = x - cw + sh;
			vertices[0].y = y - sw - ch;
			vertices[0].z = 0;
			
			vertices[1].u = srcRect->x;
			vertices[1].v = srcRect->h;
			vertices[1].x = x - cw - sh;
			vertices[1].y = y - sw + ch;
			vertices[1].z = 0;
			
			vertices[2].u = srcRect->w;
			vertices[2].v = srcRect->h;
			vertices[2].x = x + cw - sh;
			vertices[2].y = y + sw + ch;
			vertices[2].z = 0;
			
			vertices[3].u = srcRect->w;
			vertices[3].v = srcRect->y;
			vertices[3].x = x + cw + sh;
			vertices[3].y = y + sw - ch;
			vertices[3].z = 0;
			
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		}
	}
}



// 2D Shapes

void gwRender::setPrimitiveRenderMode(enum gwPrimitiveRenderMode mode){
	primitiveRenderMode = mode;
}

void gwRender::rectangle(gwRect *rect, unsigned int color, float angle){
	if(isRendering && initialized){
		if(!rect) return;
		
		sceGuDisable(GU_DEPTH_TEST);
		sceGuDepthMask(GU_TRUE);
		
		if(angle == 0.0f)
		{
			if(primitiveRenderMode == GW_OUTLINE){
				gwVertV* vertices = (gwVertV*)sceGuGetMemory((sizeof(gwVertV)*5));
				
				vertices[0].x = rect->x;//tl
				vertices[0].y = rect->y;
				vertices[0].z = 0.0f;
				
				vertices[1].x = rect->x + rect->w;//tr
				vertices[1].y = rect->y;
				vertices[1].z = 0.0f;
				
				vertices[2].x = rect->x + rect->w;//br
				vertices[2].y = rect->y + rect->h;
				vertices[2].z = 0.0f;
				
				vertices[3].x = rect->x;//bl
				vertices[3].y = rect->y + rect->h;
				vertices[3].z = 0.0f;
				
				vertices[4].x = rect->x;//tl
				vertices[4].y = rect->y;
				vertices[4].z = 0.0f;
				
				sceGuDisable(GU_TEXTURE_2D);
				sceGuColor(color);
				//sceGuShadeModel(GU_FLAT);
				sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 5, 0, vertices);
				//sceGuShadeModel(GU_SMOOTH);
				sceGuEnable(GU_TEXTURE_2D);
				
			}else{
				gwVertV* vertices = (gwVertV*)sceGuGetMemory((sizeof(gwVertV)<<1));
				
				vertices[0].x = rect->x;
				vertices[0].y = rect->y;
				vertices[0].z = 0.0f;
				
				vertices[1].x = rect->x + rect->w;
				vertices[1].y = rect->y + rect->h;
				vertices[1].z = 0.0f;
				
				sceGuDisable(GU_TEXTURE_2D);
				sceGuColor(color);
				sceGuShadeModel(GU_FLAT);
				sceGuDrawArray(GU_SPRITES, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
				sceGuShadeModel(GU_SMOOTH);
				sceGuEnable(GU_TEXTURE_2D);
				
			}
		}
		else
		{
			float x = rect->x;
			float y = rect->y;
			float width = rect->w;
			float height = rect->h;
			
			x += width * 0.5f;
			y += height * 0.5f;
			
			float c = cosf(angle), s = sinf(angle);
			
			width *= 0.5f;
			height *= 0.5f;
			
			float cw = c*width;
			float sw = s*width;
			float ch = c*height;
			float sh = s*height;
			
			gwVertV* vertices = (gwVertV*)sceGuGetMemory(5 * sizeof(gwVertV));
			
			vertices[0].x = x - cw + sh;
			vertices[0].y = y - sw - ch;
			vertices[0].z = 0;
			
			vertices[1].x = x - cw - sh;
			vertices[1].y = y - sw + ch;
			vertices[1].z = 0;
			
			vertices[2].x = x + cw - sh;
			vertices[2].y = y + sw + ch;
			vertices[2].z = 0;
			
			vertices[3].x = x + cw + sh;
			vertices[3].y = y + sw - ch;
			vertices[3].z = 0;
			
			vertices[4].x = x - cw + sh;
			vertices[4].y = y - sw - ch;
			vertices[4].z = 0;
			
			sceGuDisable(GU_TEXTURE_2D);
			sceGuColor(color);
			sceGuShadeModel(GU_FLAT);
			if(primitiveRenderMode == GW_OUTLINE)
				sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 5, 0, vertices);
			else
				sceGuDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
			sceGuShadeModel(GU_SMOOTH);
			sceGuEnable(GU_TEXTURE_2D);	
		}
	}
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);
	
}

void gwRender::gradient(gwRect *rect, unsigned int col1, unsigned int col2, unsigned int col3, unsigned int col4){
	if(initialized && isRendering){
		if(!rect) return;
		gwVertCV* vertices = (gwVertCV*)sceGuGetMemory((sizeof(gwVertCV)<<2));
		
		vertices[0].x = rect->x;//tl
		vertices[0].y = rect->y;
		vertices[0].z = 0.0f;
		vertices[0].color = col1;
		
		vertices[1].x = rect->x + rect->w;//tr
		vertices[1].y = rect->y;
		vertices[1].z = 0.0f;
		vertices[1].color = col2;

		
		vertices[2].x = rect->x + rect->w;//br
		vertices[2].y = rect->y + rect->h;
		vertices[2].z = 0.0f;
		vertices[2].color = col3;

		
		vertices[3].x = rect->x;//bl
		vertices[3].y = rect->y + rect->h;
		vertices[3].z = 0.0f;
		vertices[3].color = col4;

		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuShadeModel(GU_FLAT);
		sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, 4, 0, vertices);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);
	}
}

void gwRender::ellipse(gwRect *destRect, char steps,unsigned int color, float angle){
	if(initialized && isRendering){
		if(!destRect) return;
		
		gwVertV * vertices = (gwVertV*)sceGuGetMemory((steps + 2) * sizeof(gwVertV));

		float stepSize = (GU_PI*2)/steps;
		float w = destRect->w*0.5f;//x radius
		float h = destRect->h*0.5f;//y radius
		
		vertices[0].x = destRect->x + w ;
		vertices[0].y = destRect->y + h ;
		vertices[0].z = 0.0f;
		//vertices[0].color = color;
		
		for(int i = 1; i < steps + 2; i ++){
			//vertices[i].color = 0x00;
			vertices[i].x = destRect->x + w + sinf((i * stepSize) + angle) * w;
			vertices[i].y = destRect->y + h + cosf((i * stepSize) + angle) * h;
			vertices[i].z = 0.0f;
		}		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		if(primitiveRenderMode == GW_OUTLINE)
			sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, steps + 2, 0, vertices);
		else
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF|GU_TRANSFORM_2D, steps + 2, 0, vertices);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);	
			
	}
}

void gwRender::ellipseGradient(gwRect *destRect, char steps,unsigned int color, float angle){
	if(initialized && isRendering){
		if(!destRect) return;
		
		gwVertCV * vertices = (gwVertCV*)sceGuGetMemory((steps + 2) * sizeof(gwVertCV));
		
		float stepSize = (GU_PI*2)/steps;
		float w = destRect->w*0.5f;//x radius
		float h = destRect->h*0.5f;//y radius
		
		vertices[0].x = destRect->x + w ;
		vertices[0].y = destRect->y + h ;
		vertices[0].z = 0.0f;
		vertices[0].color = color;
		
		for(int i = 1; i < steps + 2 ; i ++){
			vertices[i].color = 0;
			vertices[i].x = destRect->x + w + sinf((i * stepSize) + angle) * w;
			vertices[i].y = destRect->y + h + cosf((i * stepSize) + angle) * h;
			vertices[i].z = 0.0f;
		}		

		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		//sceGuShadeModel(GU_FLAT);
		if(primitiveRenderMode == GW_OUTLINE)
			sceGuDrawArray(GU_LINE_STRIP, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, steps + 2, 0, vertices);
		else
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_COLOR_8888|GU_VERTEX_32BITF|GU_TRANSFORM_2D, steps + 2, 0, vertices);
		//sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);	
		
	}
}

void gwRender::star(gwRect *destRect, float innerRadius, int points, unsigned int color, float angle){
	if(initialized && isRendering){
		if(!destRect) return;
		
		angle = -angle - 90;
		
		int realPoints = points<<1;
		
		gwVertV * vertices = (gwVertV*)sceGuGetMemory((realPoints + 2) * sizeof(gwVertV));
		
		float stepSize = (GU_PI*2)/realPoints;
		int tmp;
		
		vertices[0].x = destRect->x + destRect->w*0.5f;//center
		vertices[0].y = destRect->y + destRect->h*0.5f;//center
		vertices[0].z = 0.0f;
		
		for(int i = 1; i < realPoints ; i += 2){
				vertices[i].x = destRect->x + destRect->w*0.5f + sinf((i * stepSize) + angle) * destRect->w*0.5f;
				vertices[i].y = destRect->y + destRect->h*0.5f + cosf((i * stepSize) + angle) * destRect->h*0.5f;
				vertices[i].z = 0.0f;
				
				tmp = i + 1;
			
				vertices[tmp].x = destRect->x + destRect->w*0.5f + sinf((tmp * stepSize) + angle) * innerRadius*0.5f;
				vertices[tmp].y = destRect->y + destRect->h*0.5f + cosf((tmp * stepSize) + angle) * innerRadius*0.5f;
				vertices[tmp].z = 0.0f;
		}
		
		vertices[realPoints].x = vertices[0].x;
		vertices[realPoints].y = vertices[0].y;
		vertices[realPoints].z = 0.0f;
		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		if(primitiveRenderMode == GW_OUTLINE)
			sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, realPoints + 1 , 0, vertices);
		else
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF|GU_TRANSFORM_2D,realPoints, 0, vertices);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);	
		
	}
}

void gwRender::gear(gwRect *destRect, float innerRadius, int points, unsigned int color, float angle){
	if(initialized && isRendering){
		if(!destRect) return;
		
		int realPoints = points<<1;
		
		gwVertV * vertices = (gwVertV*)sceGuGetMemory((realPoints + 2) * sizeof(gwVertV));
		
		float stepSize = (GU_PI*2)/realPoints;
		
		vertices[0].x = destRect->x + destRect->w*0.5f;//center
		vertices[0].y = destRect->y + destRect->h*0.5f;//center
		vertices[0].z = 0.0f;
		
		for(int i = 1; i < realPoints ; i ++){
			if(i & 2){
				vertices[i].x = destRect->x + destRect->w*0.5f + sinf((i * stepSize) + angle) * destRect->w*0.5f;
				vertices[i].y = destRect->y + destRect->h*0.5f + cosf((i * stepSize) + angle) * destRect->h*0.5f;
				vertices[i].z = 0.0f;
			}else{
				vertices[i].x = destRect->x + destRect->w*0.5f + sinf((i * stepSize) + angle) * innerRadius*0.5f;
				vertices[i].y = destRect->y + destRect->h*0.5f + cosf((i * stepSize) + angle) * innerRadius*0.5f;
				vertices[i].z = 0.0f;
			}
		}
		
		vertices[realPoints].x = vertices[0].x;
		vertices[realPoints].y = vertices[0].y;
		vertices[realPoints].z = 0.0f;
		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		if(primitiveRenderMode == GW_OUTLINE)
			sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, realPoints + 1 , 0, vertices);
		else
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF|GU_TRANSFORM_2D,realPoints, 0, vertices);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);	
		
	}
}

void gwRender::line(float x1, float y1, float x2, float y2, unsigned int color){
	if(initialized && isRendering){
		gwVertV* vertices = (gwVertV*)sceGuGetMemory((sizeof(gwVertV))<<1);
		
		vertices[0].x = x1;
		vertices[0].y = y1;
		vertices[0].z = 0.0f;
		
		vertices[1].x = x2;
		vertices[1].y = y2;
		vertices[1].z = 0.0f;
		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		sceGuDrawArray(GU_LINES, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 2, 0, vertices);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);
	}
}

void gwRender::point(float x, float y, unsigned int color){
	if(initialized && isRendering){
		gwVertV* vert = (gwVertV*)sceGuGetMemory((sizeof(gwVertV)));
		
		vert[0].x = x;
		vert[0].y = y;
		vert[0].z = 0.0f;
		
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		sceGuDrawArray(GU_POINTS, GU_VERTEX_32BITF|GU_TRANSFORM_2D, 1, 0, vert);
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);
	}
}

void gwRender::polygon(gwVertV *vertices, int vertCount, unsigned int color){
	if(initialized && isRendering){
		sceGuDisable(GU_TEXTURE_2D);
		sceGuColor(color);
		sceGuShadeModel(GU_FLAT);
		if(primitiveRenderMode == GW_OUTLINE)
			sceGuDrawArray(GU_LINE_STRIP, GU_VERTEX_32BITF|GU_TRANSFORM_2D, vertCount, 0, vertices);
		else
			sceGuDrawArray(GU_TRIANGLE_FAN, GU_VERTEX_32BITF|GU_TRANSFORM_2D,vertCount, 0, vertices);		
		sceGuShadeModel(GU_SMOOTH);
		sceGuEnable(GU_TEXTURE_2D);
	}
}


void gwRender::enable(enum gwGuState state){
	if(!isRendering) return;
	if(guStates & (1<<state))//already enabled
		return;
	switch (state) {
		case GW_TRIPLE_BUFFER:
			if(!done)
				done = new gwTexture(SCREEN_WIDTH, SCREEN_HEIGHT, frameFormat, GW_VRAM);
				guSwapBuffersCallback(tripleBufferCallback);
			break;
		default:
			sceGuEnable(state);//enable
			break;
	}
	guStates |= (1<<state);//add flag
}

void gwRender::disable(enum gwGuState state){
	if(!isRendering) return;
	if(!guStates & (1<<state))//already disabled
			return;
	switch (state) {//for custom states not gu supported
		case GW_TRIPLE_BUFFER:
			if(done)
				delete done;//free 3rdbuffer
			guSwapBuffersCallback(0);
			break;
		default:
			sceGuDisable(state);//enable
			break;
	}
	guStates &= ~(1<<state);//remove flag
}


unsigned int gwRender::states(){//grab states
	return guStates;
}

void gwRender::states(int states){//load states
	states &= ~(1<<GW_TRIPLE_BUFFER);//remove uneeded flag
	sceGuSetAllStatus(states);
}


//gwRender::3D
//internal matrices
void gwRender::matrixMode(enum gwMatrixMode mode){
	if(!isRendering) return;
	sceGumMatrixMode(mode);
}

void gwRender::identityMatrix(){
	if(!isRendering) return;
	sceGumLoadIdentity();
}

void gwRender::perspectiveMatrix(float fov, float aspect, float near, float far){
	if(!isRendering) return;
	sceGumPerspective(fov, aspect, near, far);
}

void gwRender::orthographicMatrix(float left, float right, float bottom, float top, float near, float far){
	if(!isRendering) return;
	sceGumOrtho(left, right, bottom, top, near, far);
}

void gwRender::lookatMatrix(gwVector3 &pos, gwVector3 &center, gwVector3 &up){
	if(!isRendering) return;
	sceGumLookAt((ScePspFVector3*)&pos, (ScePspFVector3*)&center, (ScePspFVector3*)&up);
}

void gwRender::pushMatrix(){
	if(!isRendering) return;
	sceGumPushMatrix();
}

void gwRender::popMatrix(){
	if(!isRendering) return;
	sceGumPopMatrix();
}

void gwRender::rotate(float rot, float x, float y, float z){
	if(!isRendering) return;
	if(rot == lastRot){
		multMatrix(rotMatrix);
		return;
	}
	float s = sinf(rot), c = cosf(rot);
	
	rotMatrix.x.x = x*x*(1-c)+c;
	rotMatrix.x.y = y*x*(1-c)+z*s;
	rotMatrix.x.z = x*z*(1-c)-y*s;
	rotMatrix.x.w = 0;
	
	rotMatrix.y.x = x*y*(1-c)-z*s;
	rotMatrix.y.y = y*y*(1-c)+c;
	rotMatrix.y.z = y*z*(1-c)+x*s;
	rotMatrix.y.w = 0;
	
	rotMatrix.z.x = x*z*(1-c)+y*s;
	rotMatrix.z.y = y*z*(1-c)-x*s;
	rotMatrix.z.z = z*z*(1-c)+c;
	rotMatrix.z.w = 0;
	
	rotMatrix.w.x = 0;
	rotMatrix.w.y = 0;
	rotMatrix.w.z = 0;
	rotMatrix.w.w = 1;
	
	//gl
	/*
	 rotMatrix.x.x = x*x*(1-c)+c;
	 rotMatrix.y.x = y*x*(1-c)+z*s;
	 rotMatrix.z.x = x*z*(1-c)-y*s;
	 rotMatrix.w.x = 0;
	 
	 rotMatrix.x.y = x*y*(1-c)-z*s;
	 rotMatrix.y.y = y*y*(1-c)+c;
	 rotMatrix.z.y = y*z*(1-c)+x*s;
	 rotMatrix.w.y = 0;
	 
	 rotMatrix.x.z = x*z*(1-c)+y*s;
	 rotMatrix.y.z = y*z*(1-c)-x*s;
	 rotMatrix.z.z = z*z*(1-c)+c;
	 rotMatrix.w.z = 0;
	 
	 rotMatrix.x.w = 0;
	 rotMatrix.y.w = 0;
	 rotMatrix.z.w = 0;
	 rotMatrix.w.w = 1;
	 */
	
	multMatrix(rotMatrix);
}

void gwRender::rotate(float rot, gwVector3 &axis){
	if(!isRendering) return;
	rotate(rot, axis.x, axis.y, axis.z);
}

void gwRender::rotate(gwVector3 &rot){
	if(!isRendering) return;
	sceGumRotateXYZ((ScePspFVector3*)&rot);
}


void gwRender::translate(float x, float y, float z){
	if(!isRendering) return;
	ScePspFVector3 trans = {x,y,z};
	sceGumTranslate(&trans);
}

void gwRender::translate(gwVector3 &trans){
	if(!isRendering) return;
	sceGumTranslate((ScePspFVector3*)&trans);
}

void gwRender::scale(float x, float y, float z){
	if(!isRendering) return;
	ScePspFVector3 scale = {x,y,z};
	sceGumScale(&scale);
}

void gwRender::scale(gwVector3 &scale){
	if(!isRendering) return;
	sceGumScale((ScePspFVector3*)&scale);
}



//external matrices

void gwRender::loadMatrix(gwMatrix4 &matrix){
	sceGumLoadMatrix((ScePspFMatrix4 *)&matrix);
}

void gwRender::multMatrix(gwMatrix4 &matrix){
	sceGumMultMatrix((ScePspFMatrix4 *)&matrix);
}

void gwRender::storeMatrix(gwMatrix4 &matrix){
	sceGumStoreMatrix((ScePspFMatrix4 *)&matrix);
}

void gwRender::inverseMatrix(){
	sceGumFullInverse();
}

void gwRender::inverseOrthoMatrix(){
	sceGumFastInverse();
}

void gwRender::identityMatrix(gwMatrix4 &matrix){
	gumLoadIdentity((ScePspFMatrix4 *)&matrix);
}

void gwRender::renderFullscreenTexture( float u0, float v0, float u1, float v1 )
{
	float cur_x = 0, cur_u = u0;
	float start;
	float xEnd = (float)rTarget->width;
	float slice = 64.f;
	float ustep = (u1-u0)/(float)xEnd * slice;
	
	sceGuTexWrap(GU_CLAMP,GU_CLAMP);
	sceGuDisable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_TRUE);
	for( start=0; start<xEnd; start+=slice )
	{
		gwVertTV* vertices = (gwVertTV*)sceGuGetMemory(2 * sizeof(gwVertTV));
		
		float polyWidth = ((cur_x+slice) > xEnd) ? (xEnd-cur_x) : slice;
		float sourceWidth = ((cur_u+ustep) > u1) ? (u1-cur_u) : ustep;
		
		vertices[0].u = cur_u;
		vertices[0].v = v0;
		vertices[0].x = cur_x;
		vertices[0].y = 0.f;
		vertices[0].z = 0.f;
		
		cur_u += sourceWidth;
		cur_x += polyWidth;
		
		vertices[1].u = cur_u;
		vertices[1].v = v1;
		vertices[1].x = cur_x;
		vertices[1].y = rTarget->height;
		vertices[1].z = 0.f;
		
		sceGuDrawArray(GU_SPRITES,GU_TEXTURE_32BITF|GU_VERTEX_32BITF|GU_TRANSFORM_2D,2,0,vertices);
	}
	sceGuTexWrap(GU_REPEAT,GU_REPEAT);
	sceGuEnable(GU_DEPTH_TEST);
	sceGuDepthMask(GU_FALSE);
}


void gwRender::convolution5x5(gwTexture *src, gwTexture *dst){
	if(!isRendering) return;
	
	if(!src || !dst)
		return;//missing texture
	if(src->vram() != GW_VRAM || dst->vram() != GW_VRAM)
		return;// not in vram, cannot be a rendertarget
	//else
	gwTextureS *s = src->getTextureS(), *d = dst->getTextureS();
	
	if(s->width != d->width || s->height != d->height)
		return;//images must be same size
	
	sceGuCopyImage(s->format, 0, 0, s->width, s->height, s->textureWidth, s->data, 0, 0, d->textureWidth, d->data);
	sceGuTexSync();
	
	//bind source
	src->activate();
	//second as rendertarget
	toTexture(dst);
	
	
	unsigned int twidth = d->width;
	unsigned int theight = d->height;

	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0x555555);
	float v0 = 0, v1 = theight;
	float u0 = -1.5, u1 = twidth - 1.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0xffffff);
	u0 = 1.5; u1 = twidth + 1.5;
	v0 = 0; v1 = theight;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	// renderbuffer2 -> renderbuffer1
	sceGuCopyImage(d->format, 0, 0, d->width, d->height, d->textureWidth, d->data, 0, 0, s->textureWidth, s->data);

	sceGuTexSync();
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0x555555);
	u0 = 0; u1 = twidth;
	v0 = 1.5; v1 = theight + 1.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x555555, 0xffffff);
	u0 = 0; u1 = twidth;
	v0 = -1.5; v1 = theight - 1.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	toScreen();//restore render context
}

void gwRender::convolution9x9(gwTexture *src, gwTexture *dst){
	if(!isRendering) return;
	
	if(!src || !dst)
		return;//missing texture
	if(src->vram() != GW_VRAM || dst->vram() != GW_VRAM)
		return;// not in vram, cannot be a rendertarget
	//else
	gwTextureS *s = src->getTextureS(), *d = dst->getTextureS();
	
	if(s->width != d->width || s->height != d->height)
		return;//images must be same size
	
	sceGuCopyImage(s->format, 0, 0, s->width, s->height, s->textureWidth, s->data, 0, 0, d->textureWidth, d->data);
	sceGuTexSync();

	//bind source
	src->activate();
	//second as rendertarget
	toTexture(dst);

	
	unsigned int twidth = d->width;
	unsigned int theight = d->height;
	
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0x333333);
	float u0 = -1.5, u1 = twidth - 1.5;	
	float v0 = 0, v1 = theight;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0xffffff);
	u0 = -3.5; u1 = twidth - 3.5;
	v0 = 0; v1 = theight;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 1.5; u1 = twidth + 1.5;
	v0 = 0; v1 = theight;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 3.5; u1 = twidth + 3.5;
	v0 = 0; v1 = theight;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	// renderbuffer2 -> renderbuffer1
	sceGuCopyImage(d->format, 0, 0, d->width, d->height, d->textureWidth, d->data, 0, 0, s->textureWidth, s->data);

	sceGuTexSync();
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0x333333);
	u0 = 0; u1 = twidth;
	v0 = 1.5; v1 = theight + 1.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	sceGuBlendFunc(GU_ADD, GU_FIX, GU_FIX, 0x333333, 0xffffff);	
	u0 = 0; u1 = twidth;
	v0 = 3.5; v1 = theight + 3.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0; u1 = twidth;
	v0 = -1.5; v1 = theight - 1.5;
	renderFullscreenTexture( u0, v0, u1, v1 );
	
	u0 = 0; u1 = twidth;
	v0 = -3.5; v1 = theight - 3.5;
	renderFullscreenTexture( u0, v0, u1, v1 );	
	
	sceGuBlendFunc(GU_ADD, GU_SRC_ALPHA, GU_ONE_MINUS_SRC_ALPHA, 0, 0);
	
	gwRect _R = {0,0,10,10};
	rectangle(&_R);
	
	toScreen();//restore render context
}
