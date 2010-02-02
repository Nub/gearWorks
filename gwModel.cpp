/*
 *  gwModel.cpp
 *  
 *
 *  Created by Zachry Thayer on 3/1/09.
 *  Copyright 2009 A_Nub. All rights reserved.
 *
 */

#include "gwModel.h"
#include <stdio.h>
#include <math.h>
#include <string.h>

gwModel::gwModel(const char *filePath){
	FILE *fp = fopen(filePath, "r");
	if(!fp) return;
	
	header = new gwModelHeader;
	if(!header) return;
	
	fread(header, 1, sizeof(gwModelHeader),fp);
	
	switch (header->vertType) {
		case 0x01:
			verts = (void*) new gwVertV[header->numVerts*header->numFrames];
			mesh = (void*) new gwVertV[header->numVerts];
			guFlags = (3 << 7) | (0 << 23);//GU_VERTEX_32BITF | GU_TRANSFORM_3D
			break;
		case 0x02:
			verts = (void*) new gwVertTV[header->numVerts*header->numFrames];
			mesh = (void*) new gwVertTV[header->numVerts];
			guFlags = (2 << 0) | (3 << 7) | (0 << 23);//GU_TEXTURE_16BIT | GU_VERTEX_32BITF | GU_TRANSFORM_3D
			break;
		case 0x04:
			verts = (void*) new gwVertTNV[header->numVerts*header->numFrames];
			mesh = (void*) new gwVertTNV[header->numVerts];
			guFlags = (2 << 0) | (1 << 5) | (3 << 7) | (0 << 23);//GU_TEXTURE_16BIT | GU_NORMAL_8BIT | GU_VERTEX_32BITF | GU_TRANSFORM_3D
			break;
		default:
			break;
	}
	
	fread(verts, header->numVerts, header->numFrames, fp);
	
	char buffer[512];
	int pathLen = strlen(filePath);
	for(int i = 0; i < pathLen-3; i ++){
		buffer[i] = filePath[i];
	}
	strcat(buffer,"png");

	tex = new gwTexture(buffer);
	
	fclose(fp);
}

gwModel::~gwModel(){
	delete header;
	delete tex;
	
	int *tmp = (int*)verts;
	delete tmp;
	tmp = (int*)mesh;
	delete tmp;
}

int gwModel::getVertCount(){
	return header->numVerts;
}

int gwModel::getGuFlags(){
	return guFlags;
}

void gwModel::setFrame(unsigned short newFrame){
	frame = newFrame;
}

void gwModel::interpolate(float weight){
	if(weight >= 1.0f){
		frame ++;
		while(weight > 1.0f)
			weight -= 1.0f;//keep in range of 0 - 1
	}
	
	int nextFrame = frame + 1;
	if(nextFrame > header->numFrames)
		return;

	switch (header->vertType) {
		case 0x01:
		{
			gwVertV *thisMesh = (gwVertV*)(&verts + (header->numVerts*frame*sizeof(gwVertV)));
			gwVertV *nextMesh = (gwVertV*)(&verts + (header->numVerts*(nextFrame)*sizeof(gwVertV)));
			gwVertV *newMesh = (gwVertV*)mesh;
			for(int i = 0; i < header->numVerts; i ++){
				newMesh[i].x = thisMesh[i].x + (fabs(thisMesh[i].x) - fabs(nextMesh[i].x)) * weight;
				newMesh[i].y = thisMesh[i].y + (fabs(thisMesh[i].y) - fabs(nextMesh[i].y)) * weight;
				newMesh[i].z = thisMesh[i].z + (fabs(thisMesh[i].z) - fabs(nextMesh[i].z)) * weight;
			}
			
			break;
		}
		case 0x02:
		{
			gwVertTV *thisMesh = (gwVertTV*)(&verts + (header->numVerts*frame*sizeof(gwVertTV)));
			gwVertTV *nextMesh = (gwVertTV*)(&verts + (header->numVerts*(nextFrame)*sizeof(gwVertTV)));
			gwVertTV *newMesh = (gwVertTV*)mesh;
			for(int i = 0; i < header->numVerts; i ++){
				newMesh[i].u = thisMesh[i].u;
				newMesh[i].v = thisMesh[i].v;

				newMesh[i].x = thisMesh[i].x + (fabs(thisMesh[i].x) - fabs(nextMesh[i].x)) * weight;
				newMesh[i].y = thisMesh[i].y + (fabs(thisMesh[i].y) - fabs(nextMesh[i].y)) * weight;
				newMesh[i].z = thisMesh[i].z + (fabs(thisMesh[i].z) - fabs(nextMesh[i].z)) * weight;
			}
			
			break;
		}
		case 0x04:
		{
			gwVertTNV *thisMesh = (gwVertTNV*)(&verts + (header->numVerts*frame*sizeof(gwVertTNV)));
			gwVertTNV *nextMesh = (gwVertTNV*)(&verts + (header->numVerts*(nextFrame)*sizeof(gwVertTNV)));
			gwVertTNV *newMesh = (gwVertTNV*)mesh;

			for(int i = 0; i < header->numVerts; i ++){
				newMesh[i].u = thisMesh[i].u;
				newMesh[i].v = thisMesh[i].v;
				
				newMesh[i].nx = thisMesh[i].nx;
				newMesh[i].ny = thisMesh[i].ny;
				newMesh[i].nz = thisMesh[i].nz;
				
				newMesh[i].x = thisMesh[i].x + (fabs(thisMesh[i].x) - fabs(nextMesh[i].x)) * weight;
				newMesh[i].y = thisMesh[i].y + (fabs(thisMesh[i].y) - fabs(nextMesh[i].y)) * weight;
				newMesh[i].z = thisMesh[i].z + (fabs(thisMesh[i].z) - fabs(nextMesh[i].z)) * weight;
			}
			
			break;
		}
		default:
			break;
	}
	
}

void gwModel::save(const char *filePath,gwModelHeader *Header, void *Verts){
	FILE *fp = fopen(filePath, "w");
	if(!fp) return;
	
	fwrite(Header, 1, sizeof(gwModelHeader),fp);
	
	int vertSize;
	
	switch (Header->vertType) {
		case 0x01:
			vertSize = 3 * sizeof(float);
			break;
		case 0x02:
			vertSize = (3 * sizeof(float)) + (2 * sizeof(float));
			break;
		case 0x04:
			vertSize = (3 * sizeof(float)) + (2 * sizeof(float)) + (3 * sizeof(char));
			break;
		default:
			break;
	}
	fwrite(Verts, Header->numVerts * vertSize, Header->numFrames, fp);
	
	fclose(fp);
}
