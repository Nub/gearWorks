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
		
		gwVector::( float InX, float InY, float InZ ) : x( InX ), y( InY ), z( InZ ){}
		gwVector::( ) : x(0), y(0), z(0){}
		
		inline bool gwVector::operator== (const gwVector::& V2) const 
		{
			return (x == V2.x && y == V2.y && z == V2.z);
		}
		
		inline gwVector gwVector::operator+ (const gwVector::& V2) const 
		{
			return gwVector( x + V2.x,  y + V2.y,  z + V2.z);
		}
		inline gwVector gwVector:: operator- (const gwVector::& V2) const
		{
			return gwVector( x - V2.x,  y - V2.y,  z - V2.z);
		}
		inline gwVector gwVector:: operator- ( ) const
		{
			return gwVector(-x, -y, -z);
		}
		
		inline gwVector gwVector:: operator/ (float S ) const
		{
			float fInv = 1.0f / S;
			return gwVector(x * fInv , y * fInv, z * fInv);
		}
		inline gwVector gwVector:: operator/ (const gwVector::& V2) const
		{
			return gwVector(x / V2.x,  y / V2.y,  z / V2.z);
		}
		inline gwVector gwVector:: operator* (const gwVector::& V2) const
		{
			return gwVector(x * V2.x,  y * V2.y,  z * V2.z);
		}
		inline gwVector gwVector:: operator* (float S) const
		{
			return gwVector(x * S,  y * S,  z * S);
		}
		
		inline void operator+= ( const gwVector::& V2 )
		{
			x += V2.x;
			y += V2.y;
			z += V2.z;
		}
		inline void operator-= ( const gwVector::& V2 )
		{
			x -= V2.x;
			y -= V2.y;
			z -= V2.z;
		}
		
		inline float operator[] ( int i )
		{
			if ( i == 0 ) return x;
			else if ( i == 1 ) return y;
			else return z;
		}
		
		inline float gwVector::dot( const gwVector:: &V1 ) const
		{
			return V1.x*x + V1.y*y + V1.z*z;
		}
		
		inline gwVector gwVector::crossProduct( const gwVector:: &V2 ) const
		{
			return gwVector(
						 y * V2.z  -  z * V2.y,
						 z * V2.x  -  x * V2.z,
						 x * V2.y  -  y * V2.x 	);
		}
		
		gwVector gwVector::rotByMatrix( const float m[16] ) const
		{
			return gwVector( 
						 x*m[0] + y*m[4] + z*m[8],
						 x*m[1] + y*m[5] + z*m[9],
						 x*m[2] + y*m[6] + z*m[10] );
		}
		
		float gwVector::magnitude( ) const
		{
			return sqrtf( x*x + y*y + z*z );
		}
		
		float gwVector::distance( const gwVector:: &V1 ) const
		{
			return ( *this - V1 ).Magnitude();	
		}
		
		inline void Normalize()
		{
			float fMag = ( x*x + y*y + z*z );
			if (fMag == 0) {return;}
			
			float fMult = 1.0f/sqrtf(fMag);            
			x *= fMult;
			y *= fMult;
			z *= fMult;
			return;
		}
	};
#endif

