/*************************************************************************\
 Engine: gearWorks
 File: gwObj.h
 Author: Zachry Thayer
 Description: Loads and renders and Wavefront OBJ
 Requires: 
\*************************************************************************/

#ifndef _GWOBJ_H
#define _GWOBJ_H

#include "gwTypes.h"
#include "gwTexture.h"
#include <vector>
#include <stdio.h>
#include <pspgu.h>
#include <pspgum.h>
#include <pspkernel.h>


class gwObj{
private:
	std::vector<gwTexture *>tex;
	std::vector<gwVertV> verts;
	std::vector<gwVertV> normals;
	std::vector<gwPoint2f> uvs;
	std::vector<unsigned int> faces; // v/t/n 
	
	unsigned int meshSize;
	gwVertTNV *mesh;
	
	void loadFromFile(char *file);
	void parseObjData(char *data, unsigned int size);
	gwVertTNV *buildMesh();
	
	float uScale, vScale;
		
public:
	gwObj(char *file);
	gwObj(char *data, unsigned int size);
	~gwObj();
	void bindTexture(gwTexture *tex);
	void setTexScale(float u, float v);
	void render();
};


#endif
