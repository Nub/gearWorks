/*************************************************************************\
 Engine: gearWorks
 File: gwModel.cpp
 Author: Zachry Thayer
 Description: Loads and Displays custom model format
 Requires:
\*************************************************************************/
#ifndef _GWMODEL_H
#define _GWMODEL_H

#include "gwTypes.h"
#include "gwTexture.h"

class gwModel{
private:
	gwModelHeader *header;//info
	gwTexture *tex;//skin
	void *verts;//all the frames
	unsigned short frame;//current frame
	int guFlags;
	
public:
	gwModel(const char *filePath);
	~gwModel();
	
	void *mesh;//interpolation populates this for rendering
	
	int getVertCount();//returns frame size
	int getGuFlags();//returns draw array flags
	
	void setFrame(unsigned short newFrame);//to jump to a different frame
	void interpolate(float weight);//when weight  >= 1 frame ++
	void render();//init texture and call draw array
	
	static void save(const char *filePath, gwModelHeader *Header, void *Verts);//write to a file
	
};

#endif
