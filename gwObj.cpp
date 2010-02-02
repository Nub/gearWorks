/*************************************************************************\
 Engine: gearWorks
 File: gwObj.cpp
 Author: Zachry Thayer
 Description: Loads and renders and Wavefront OBJ
 Requires: 
\*************************************************************************/

#include "gwObj.h"

gwObj::gwObj(char *file){
	
	uScale = 1.f;
	vScale = 1.f;
		
	loadFromFile(file);
	mesh = buildMesh();
	//sceKernelCacheWriteBackInvalidateAll();
}

gwObj::gwObj(char *data, unsigned int size){
	
	uScale = 1.f;
	vScale = 1.f;
	
	parseObjData(data,size);
	mesh = buildMesh();
}

gwObj::~gwObj(){
	if(mesh)
		free(mesh);
	else{
		verts.clear();
		uvs.clear();
		normals.clear();
		faces.clear();
	}
}

void gwObj::loadFromFile(char *file){
	FILE *fp = fopen(file, "r");
	if(!fp){
		//error
	//	assert(fp);
	}
	fseek(fp, 0, SEEK_END);
	long size = ftell(fp);
	rewind(fp);
	
	char *data = new char[size];
	
	fread(data, size, 1, fp);
	if(ferror(fp)){
		//assert(ferror(fp));
	}
	
	//load texture;
	//tex = new gwTexture(file);
	
	parseObjData(data, size);
	
}

void gwObj::parseObjData(char *data, unsigned int size){
	char *p = data;
	char p2[255];
	char *end = p + size;
	char c,cc;
	
	gwVertV vtmp;
	gwPoint2f ptmp;
	unsigned int face[9] = {0,0,0,0,0,0,0,0,0};
	
	while(p < end){// while still in the buffer limit
		while(*p == ' ')p++;//skip whitespace
		if(*p == '#')while(*p != '\n')p++;//jump to next line
		
		c = p[0];
		cc = p[1];
		
		for(int i = 0; i < 255; i++){
			if(p[i] != '\n')
				p2[i] = p[i];
			else{
				p2[i] = 0;
				break;
			}
		}
		
		switch (c) {
			case 'v':
				switch(cc){
					case 'n'://normal
						sscanf(p2, "vn %f %f %f", &vtmp.x, &vtmp.y, &vtmp.z);
						normals.push_back(vtmp);
					//	printf("vn %f %f %f\n" , vtmp.x, vtmp.y, vtmp.z);
						break;
					case 't':
						sscanf(p2, "vt %f %f", &ptmp.x, &ptmp.y);
						uvs.push_back(ptmp);
					//	printf("vt %f %f %f\n" , ptmp.x, ptmp.y);
						break;
						
					case ' ':
						sscanf(p2, "v %f %f %f", &vtmp.x, &vtmp.y, &vtmp.z);
						verts.push_back(vtmp);
					//	printf("v %f %f %f\n" , vtmp.x, vtmp.y, vtmp.z);
						break;
				}
				break;
			case 'f':
				sscanf(p2, "f %d/%d/%d %d/%d/%d %d/%d/%d", &face[0], &face[1], &face[2], &face[3], &face[4], &face[5], &face[6], &face[7], &face[8]);
				for(int i = 0; i < 9; i ++)
					faces.push_back(face[i]);
				//printf("f %d/%d/%d %d/%d/%d %d/%d/%d\n", face[0], face[1], face[2], face[3], face[4], face[5], face[6], face[7], face[8]);
				break;
			}
		while(*p++ != '\n');
	}
}


gwVertTNV* gwObj::buildMesh(){
	meshSize = faces.size()/3;
	gwVertTNV *ret = new gwVertTNV[meshSize];
	for(unsigned int i = 0, c = 0; i < faces.size()-1; i ++, c++){
		ret[c].x = verts[faces[i]-1].x;
		ret[c].y = verts[faces[i]-1].y;
		ret[c].z = verts[faces[i]-1].z;
		i++;
		ret[c].u = uvs[faces[i]-1].x;
		ret[c].v = 1 - uvs[faces[i]-1].y;
		i++;
		if(normals.size() != 0){
			ret[c].nx = normals[faces[i]-1].x;
			ret[c].ny = normals[faces[i]-1].y;
			ret[c].nz = normals[faces[i]-1].z;
		}else{
			ret[c].nx = 1;
			ret[c].ny = 1;
			ret[c].nz = 1;
		}
	}
	
	verts.clear();
	uvs.clear();
	normals.clear();
	faces.clear();
	sceKernelDcacheWritebackInvalidateAll();
#ifdef GW_VERBOSE
	printf("Mesh Built, with %d vertices [%fkb]\n", meshSize, (float)(sizeof(gwVertTNV) * meshSize)/1024.f);
#endif
	return ret;
}

void gwObj::bindTexture(gwTexture *texture){
	tex.push_back(texture);
}

void gwObj::setTexScale(float u, float v){
	uScale = u;
	vScale = v;
}

void gwObj::render(){
	//if(tex){
	//	tex->activate();
	//}
	if (tex.size() > 1)
		for(int i = 0; i < tex.size(); i ++){
			tex[i]->activate(i);
		}
	else tex[0]->activate();
	
	sceGuTexScale(uScale, vScale);
	sceGuTexFunc(GU_TFX_MODULATE, GU_TCC_RGBA);
	//sceGuDisable(GU_TEXTURE_2D);
	sceGumDrawArray(GU_TRIANGLES, GU_VERTEX_32BITF | GU_TEXTURE_32BITF | GU_NORMAL_32BITF | GU_TRANSFORM_3D, meshSize, 0, mesh);
	//sceGuEnable(GU_TEXTURE_2D);
	/*for(int i = 0; i < meshSize; i++){
		printf("Vert[%d]: %f, %f\n",i, mesh[i].u, mesh[i].v);
	}*/
}
