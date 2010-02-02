/*************************************************************************\
 Engine: gearWorks
 File: gwQuaternion.h
 Author: Zachry Thayer
 Description: implements quaternions
 Requires:
\*************************************************************************/

#ifndef _GWQUATERNION_H
#define _GWQUATERNION_H

#include <math.h>

#define PI			3.14159265358979323846

class gwQuaternion  
{
public:
	gwQuaternion operator *(gwQuaternion q);
	void createMatrix(float *pMatrix);
	void createFromAxisAngle(float x, float y, float z, float degrees);
	gwQuaternion();
	~gwQuaternion();

private:
	float m_w;
	float m_z;
	float m_y;
	float m_x;
};

#endif
