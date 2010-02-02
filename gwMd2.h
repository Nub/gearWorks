/*************************************************************************\
 Engine: gearWorks
 File: gwMd2.h
 Author: Zachry Thayer
 Description: Loads, animates, and displayes md2 models
 Requires: gwTexture gwRender
\*************************************************************************/
#ifndef _GWMD2_H
#define _GWMD2_H

#include "gwTypes.h"
#include "gwTexture.h"

#include <stdio.h>

class gwMd2{
private:
	gwMd2Header *header;//file info
	gwMd2Skin *skin;//list of skins
	gwMd2TexCoord *texCoord;//list of U,V's
	gwMd2Triangle *triangle;//list of triangles
	gwMd2Frame *frame;//list of frames
	int *glcmds;
	unsigned int textureId;//which skin to use
	bool isStatic;//if one frame we are a static model and can save some resources
	gwTexture *tex;//the actual texture
	 
	//helper functions
	gwMd2Header *loadHeader(FILE *fp);
	gwMd2Skin *loadSkins(FILE *fp);
	gwMd2TexCoord *loadTexCoords(FILE *fp);
	gwMd2Triangle *loadTriangles(FILE *fp);
	gwMd2Frame *loadFrames(FILE *fp);
	int *loadGlCommands(FILE *fp);
	
public:
	gwMd2(const char *filePath);
	~gwMd2();
	
};

#endif

