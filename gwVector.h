/*
 *  Vector.h
 *  
 *
 *  Created by Zachry Thayer on 1/27/09.
 *  Copyright 2009 A_Nub. All rights reserved.
 *
 */

#ifndef _GWVECTOR_H
#define _GWVECTOR_H

#include <math.h>

class gwVector{
	public:
		float x, y, z;
		
		gwVector(float InX = 0.f,float InY = 0.f,float InZ = 0.f);
		
		inline float dot( const gwVector &V1 );
		
		inline Vector3 crossProduct( const gwVector &V2 );
		
		gwVector rotByMatrix( const float m[16] );
		
		float magnitude();
		
		float distance( const gwVector &V1 );
		
		inline void normalize();
};
#endif

