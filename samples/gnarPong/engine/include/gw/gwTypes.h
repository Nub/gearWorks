/*************************************************************************\
 Engine: gearWorks
 File: gwTypes.h
 Author: Zachry Thayer
 Description: include and global definition file
 \*************************************************************************/
#ifndef _GW_TYPES_H
#define _GW_TYPES_H

enum gwPixelFormat
{
	GW_PIXEL_FORMAT_5650,	/**< Pixelformat R5:G6:B5:A0. */
	GW_PIXEL_FORMAT_5551,	/**< Pixelformat R5:G5:B5:A1. */
	GW_PIXEL_FORMAT_4444,	/**< Pixelformat R4:G4:B4:A4. */
	GW_PIXEL_FORMAT_8888,	/**< Pixelformat R8:G8:B8:A8. */
	GW_PIXEL_FORMAT_T4,	/**< Pixelformat 4bit indexed. */
	GW_PIXEL_FORMAT_T8,	/**< Pixelformat 8bit indexed. */
	GW_PIXEL_FORMAT_T16,	/**< Pixelformat 16bit indexed. */
	GW_PIXEL_FORMAT_T32	/**< Pixelformat 32bit indexed. */
};

enum gwProjectionMode{
	GW_PROJECTION_2D,
	GW_PROJECTION_ORTHO,
	GW_PROJECTION_3D
};

enum gwTextureMode
{
	GW_TEX_MODE_MODULATE,
	GW_TEX_MODE_DECAL,
	GW_TEX_MODE_BLEND,
	GW_TEX_MODE_REPLACE,
	GW_TEX_MODE_ADD
};

enum gwMemoryLocation
{
	GW_RAM,
	GW_VRAM
};

enum gwPrimitiveRenderMode{
	GW_OUTLINE,
	GW_FILL
};

typedef struct  {
	void					*palette;		/**< Image palette. */
	enum gwPixelFormat		palFormat;		/**< Palette format - one of GW_PIXEL_FORMAT_5650, GW_PIXEL_FORMAT_5551, GW_PIXEL_FORMAT_4444, GW_PIXEL_FORMAT_8888. */
	void					*data;			/**< Image data. */
	unsigned int			size;			/**< Size of data in bytes. */
	unsigned int			width;			/**< Image width. */
	unsigned int			height;			/**< Image height. */
	unsigned int			textureWidth;	/**< Texture width (power of two). */
	unsigned int			textureHeight;	/**< Texture height (power of two). */
	unsigned int			bits;			/**< Image bits per pixel. */
	enum gwPixelFormat		format;			/**< Image format - one of ::gwPixelFormat. */
	char					swizzled;		/**< Is image swizzled. */
	enum gwMemoryLocation	location;		/**< One of ::gwMemoryLocation. */
	
} gwTextureS;

typedef struct
{
	float	x, y, z;
} gwVertV, gwPoint;

typedef struct
{
	float	nx, ny, nz;
	float	x, y, z;
} gwVertNV;

typedef struct
{
	unsigned int	color;
	float			x, y, z;
} gwVertCV;

typedef struct
{
	unsigned int	color;
	float			nx, ny, nz;
	float			x, y, z;
} gwVertCNV;

typedef struct
{
	float	u, v;
	float	x, y, z;
} gwVertTV;

typedef struct
{
	float	u, v;
	char	nx, ny, nz;
	float	x, y, z;
} gwVertTNV;

typedef struct
{
	float			u, v;
	unsigned int	color;
	float			x, y, z;
} gwVertTCV;

typedef struct
{
	float			u, v;
	unsigned int	color;
	float			nx, ny, nz;
	float			x, y, z;
} gwVertTCNV;

typedef struct
{
	float	x, y, w, h;
} gwRect;


typedef struct _gw_model_header {
	int identifier; // "GWMF" for Gear Works Model Format
	int numVerts;//number of vertices perframe
	int numFrames;//number of frames
	unsigned char vertType;//what kind of vertex?
} gwModelHeader;


/* File Header */
typedef struct md2_header {
	int magic; // "IDP2"
	int version; // always 8
	int skinWidth; //width if texture in pixels
	int skinHeight; //height of texture in pixels
	int frameSize; //size of each frame in bytes
	int numSkins; //how many textures?
	int numVertices; //numbers of vertices in each frame
	int numTexCoords; // not always the same as vertices
	int numTriangles; //number of triangles per frame
	int numGlCommands; //number of dwords in the glDispList
	int numFrames; // number of frames (1 = no anim)
	int offsetSkins; // offset to list of skin names
	int offsetTexCoords; // offset to list of tex co-ords
	int offsetTriangles; // offset  to list of triangles
	int offsetFrames; //list of triangles
	int offsetGlCommands; //list of glCOmmands
	int offsetEnd; // file size
} gwMd2Header;

/* Texture name */
typedef struct md2_skin
{
	char name[64];
}gwMd2Skin;

/* Texture coords */
typedef struct md2_texCoord
{
	short s;
	short t;
}gwMd2TexCoord;

/* Triangle info */
typedef struct md2_triangle
{
	unsigned short vertex[3];
	unsigned short st[3];
}gwMd2Triangle;

/* Compressed vertex */
typedef struct md2_vertex
{
	unsigned char v[3];
	unsigned char normalIndex;
}gwMd2Vertex;

/* Model frame */
typedef struct md2_frame
{
	float scale[3];
	float translate[3];
	char name[16];
	gwMd2Vertex *verts;
}gwMd2Frame;

/* GL command packet */
typedef struct md2_glcmd
{
	float s;
	float t;
	int index;
}gwMd2GlCmd;



#endif
