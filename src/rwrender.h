namespace rw {

// Render states

enum RenderState
{
	VERTEXALPHA = 0,
	SRCBLEND,
	DESTBLEND,
	ZTESTENABLE,
	ZWRITEENABLE,
	FOGENABLE,
	FOGCOLOR,
	// TODO:
	// fog type, density ?
	// ? cullmode
	// ? shademode
	// ???? stencil

	// platform specific or opaque?
	ALPHATESTFUNC,
	ALPHATESTREF,
};

enum AlphaTestFunc
{
	ALPHAALWAYS = 0,
	ALPHAGREATEREQUAL,
	ALPHALESS
};

enum BlendFunction
{
	BLENDZERO = 0,
	BLENDONE,
	BLENDSRCCOLOR,
	BLENDINVSRCCOLOR,
	BLENDSRCALPHA,
	BLENDINVSRCALPHA,
	BLENDDESTALPHA,
	BLENDINVDESTALPHA,
	BLENDDESTCOLOR,
	BLENDINVDESTCOLOR,
	BLENDSRCALPHASAT,
	// TODO: add more perhaps
};

void SetRenderState(int32 state, uint32 value);
uint32 GetRenderState(int32 state);

// Im2D

namespace im2d {

float32 GetNearZ(void);
float32 GetFarZ(void);
void RenderIndexedPrimitive(PrimitiveType, void *verts, int32 numVerts, void *indices, int32 numIndices);

}

// Im3D

namespace im3d {

void Transform(void *vertices, int32 numVertices, Matrix *world);
void RenderIndexed(PrimitiveType primType, void *indices, int32 numIndices);
void End(void);

}

}

