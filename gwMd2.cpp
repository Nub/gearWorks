/*************************************************************************\
 Engine: gearWorks
 File: gwMd2.cpp
 Author: Zachry Thayer
 Description: Loads, animates, and displayes md2 models
 Requires: gwTexture gwRender
 \*************************************************************************/
#include "gwMd2.h"
#include <assert.h>

gwMd2::gwMd2(const char *filePath){
	FILE *fp = fopen(filePath,"r");
	assert(fp);	
	header = loadHeader(fp);
	assert(header);
	skin = loadSkins(fp);
	assert(skin);
	texCoord = loadTexCoords(fp);
	assert(texCoord);
	triangle = loadTriangles(fp);
	assert(triangle);
	frame = loadFrames(fp);
	assert(frame);
	glcmds = loadGlCommands(fp);
	assert(glcmds);
	
	tex = new gwTexture(skin[textureId].name);
	assert(tex);
	
	fclose(fp);
}

gwMd2::~gwMd2(){
	delete skin;
	delete texCoord;
	delete triangle;
	for(int i = 0; i < header->numFrames; i ++){
		delete frame->verts;
	}
	delete frame;
	delete header;
	delete glcmds;
}

gwMd2Header *gwMd2::loadHeader(FILE *fp){
	rewind(fp);//precaution
	
	gwMd2Header *ret = new gwMd2Header;
	if(!ret) return NULL;//failed to allocate space
	
	fread(ret, 1, sizeof(gwMd2Header), fp);//read in header
	
	if(ret->magic != 844121161 && ret->version != 8)//bad version or ID
		return NULL;
	
	if(ret->numFrames == 1) isStatic = true;// do we have a static model
	else isStatic = false;

	return ret;
}

gwMd2Skin *gwMd2::loadSkins(FILE *fp){
	if(!header) return NULL;
	rewind(fp);//precaution
	
	gwMd2Skin *ret = new gwMd2Skin[header->numSkins];
	if(!ret) return NULL;//failed to allocate space
	
	fseek(fp, header->offsetSkins, SEEK_SET);//move the fp to the correct spot	
	fread(ret, header->numSkins, sizeof(gwMd2Skin), fp);//read in the skin data
	
	textureId = 0;// use the first skin
	
	return ret;
}

gwMd2TexCoord *gwMd2::loadTexCoords(FILE *fp){
	if(!header) return NULL;
	rewind(fp);//precaution
	
	gwMd2TexCoord *ret = new gwMd2TexCoord[header->numTexCoords];
	if(!ret) return NULL;//failed to allocate data
	
	fseek(fp, header->offsetTexCoords, SEEK_SET);//move the fp to the correct spot	
	fread(ret, header->numTexCoords, sizeof(gwMd2TexCoord), fp);//read in the skin data
	
	return ret;
}

gwMd2Triangle *gwMd2::loadTriangles(FILE *fp){
	if(!header) return NULL;
	rewind(fp);//precaution
	
	gwMd2Triangle *ret = new gwMd2Triangle[header->numTriangles];
	if(!ret) return NULL;//failed to allocate data
	
	fseek(fp, header->offsetTriangles, SEEK_SET);//move the fp to the correct spot	
	fread(ret, header->numTriangles, sizeof(gwMd2Triangle), fp);//read in the skin data
	
	return ret;
}

gwMd2Frame *gwMd2::loadFrames(FILE *fp){
	if(!header) return NULL;
	rewind(fp);//precaution
	
	gwMd2Frame *ret = new gwMd2Frame[header->numFrames];
	if(!ret) return NULL;
	
	fseek (fp, header->offsetFrames, SEEK_SET);
	
	for (int i = 0; i < header->numFrames; i ++){
		ret[i].verts = new gwMd2Vertex[header->numVertices];
		if(!ret[i].verts){
			for(int j = i; j >= 0; j --){
				delete ret[j].verts;
			}
			delete ret;
			return NULL;
		}
		fread(ret[i].scale, sizeof(float), 3, fp);
		fread(ret[i].translate, sizeof(float), 3, fp);
		fread(ret[i].name, sizeof(char), 16, fp);
		fread(ret[i].verts, sizeof(gwMd2Vertex), header->numVertices, fp);
	}
	
	return ret;
}

int *gwMd2::loadGlCommands(FILE *fp){
	if(!header) return NULL;
	rewind(fp);//precaution
	
	int *ret = new int[header->numGlCommands];
	if(!ret) return NULL;//failed to allocate data
	
	fseek(fp, header->offsetGlCommands, SEEK_SET);//move the fp to the correct spot	
	fread(ret, header->numGlCommands, sizeof(int), fp);//read in the skin data
	
	return ret;
}
