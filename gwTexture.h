/*************************************************************************\
 Engine: gearWorks
 File: gwTexture.h
 Author: Zachry Thayer
 Description: Loads png images and saves them
 Requires: -lpng -lz
\*************************************************************************/
#ifndef _GW_TEXTURE_H
#define _GW_TEXTURE_H

#include "gwTypes.h"

class gwTexture{
private:
	gwTextureS *texture;
	
	gwTextureS* load(const char *filename, enum gwMemoryLocation location, unsigned char swizzle);
	gwTextureS* loadMemory(unsigned char *buffer, int size, enum gwMemoryLocation location, unsigned char swizzle);
	gwTextureS* create(unsigned int width, unsigned int height, enum gwPixelFormat format, enum gwMemoryLocation location);

	void gwTexturePaletteGet(unsigned int col, unsigned int *r, unsigned int *g, unsigned int *b, unsigned int *a);
	
	void destroy();
	
public:
	
	gwTexture(const char *filename, int memLocation = GW_RAM, bool swizzle = true);
	gwTexture(int width, int height, int pixelFormat = GW_PIXEL_FORMAT_8888, int memLocation = GW_RAM);
	gwTexture(gwTextureS *tex);
	~gwTexture();
	
	bool isValid();//checks to make sure the image properly loaded
	
	gwTextureS* getTextureS();
	
	int swizzle();
	int unswizzle();
	unsigned int getPixel(unsigned int x, unsigned int y);
	void setPixel(unsigned int color, unsigned int x, unsigned int y);
	void setAllPixels(unsigned int color);
	bool activate();
	bool activate(int mip);
	int toRam();
	int toVram();
	enum gwMemoryLocation vram();
	int modeSet(enum gwTextureMode mode);
	void save(const char* filename);
	void screenshot(const char* filename);
	
};

#endif
