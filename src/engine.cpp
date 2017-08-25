#include <cstdio>
#include <cstdlib>
#include <cassert>

#include "rwbase.h"
#include "rwerror.h"
#include "rwplg.h"
#include "rwpipeline.h"
#include "rwobjects.h"
#include "rwengine.h"
#include "ps2/rwps2.h"
#include "d3d/rwxbox.h"
#include "d3d/rwd3d.h"
#include "d3d/rwd3d8.h"
#include "d3d/rwd3d9.h"
#include "gl/rwgl3.h"
#include "gl/rwwdgl.h"

#define PLUGIN_ID 0

// on windows
//#ifdef DEBUG
//#include <crtdbg.h>
//#define free(p) _free_dbg(p, _NORMAL_BLOCK);
//#define malloc(sz) _malloc_dbg(sz, _NORMAL_BLOCK, __FILE__, __LINE__)
//#define realloc(p, sz) _realloc_dbg(p, sz, _NORMAL_BLOCK, __FILE__, __LINE__)
//#endif

namespace rw {

Engine *engine;
PluginList Driver::s_plglist[NUM_PLATFORMS];
Engine::State Engine::state = Dead;
MemoryFunctions Engine::memfuncs;

void *malloc_h(size_t sz, uint32 hint) { if(sz == 0) return nil; return malloc(sz); }
void *realloc_h(void *p, size_t sz, uint32 hint) { return realloc(p, sz); }
// TODO: make the debug out configurable
void *mustmalloc_h(size_t sz, uint32 hint)
{
	void *ret;
	ret = rwMalloc(sz, hint);
	if(ret)
		return ret;
	fprintf(stderr, "Error: out of memory\n");
	exit(1);
	return nil;
}
void *mustrealloc_h(void *p, size_t sz, uint32 hint)
{
	void *ret;
	ret = rwRealloc(p, sz, hint);
	if(ret)
		return ret;
	fprintf(stderr, "Error: out of memory\n");
	exit(1);
	return nil;
}

// This function mainly registers engine plugins
// RW initializes memory related things here too and
// uses more plugins
// TODO: do this^ ?
bool32
Engine::init(void)
{
	if(engine || Engine::state != Dead){
		RWERROR((ERR_ENGINEINIT));
		return 0;
	}

	memfuncs.rwmalloc = malloc_h;
	memfuncs.rwrealloc = realloc_h;
	memfuncs.rwfree = free;
	memfuncs.rwmustmalloc = mustmalloc_h;
	memfuncs.rwmustrealloc = mustrealloc_h;

	PluginList init = { sizeof(Driver), sizeof(Driver), nil, nil };
	for(uint i = 0; i < NUM_PLATFORMS; i++)
		Driver::s_plglist[i] = init;

	// Register plugins
	// TODO: these are wrong
	ps2::initializePlatform();
	xbox::initializePlatform();
	d3d8::initializePlatform();
	d3d9::initializePlatform();
	wdgl::initializePlatform();
	gl3::initializePlatform();

	Engine::state = Initialized;
	return 1;
}

// This is where RW allocates the engine and e.g. opens d3d
// TODO: this will take an argument with device specific data (HWND &c.)
bool32
Engine::open(void)
{
	if(engine || Engine::state != Initialized){
		RWERROR((ERR_ENGINEOPEN));
		return 0;
	}

	// Allocate engine
	engine = rwNewT(Engine, 1, MEMDUR_GLOBAL);
	engine->currentCamera = nil;
	engine->currentWorld = nil;
	engine->currentTexDictionary = nil;
	engine->imtexture = nil;
	engine->loadTextures = 1;
	engine->makeDummies = 1;

	// Initialize device
	// Device and possibly OS specific!
#ifdef RW_PS2
	engine->device = ps2::renderdevice;
#elif RW_GL3
	engine->device = gl3::renderdevice;
#elif RW_D3D9
	engine->device = d3d::renderdevice;
#else
	engine->device = null::renderdevice;
#endif

	// TODO: open device; create d3d object/get video mode

	// TODO: init driver functions
	ObjPipeline *defpipe = new ObjPipeline(PLATFORM_NULL);
	for(uint i = 0; i < NUM_PLATFORMS; i++){
		rw::engine->driver[i] = (Driver*)rwNew(Driver::s_plglist[i].size,
			MEMDUR_GLOBAL);

		engine->driver[i]->defaultPipeline = defpipe;

		engine->driver[i]->rasterCreate = null::rasterCreate;
		engine->driver[i]->rasterLock = null::rasterLock;
		engine->driver[i]->rasterUnlock = null::rasterUnlock;
		engine->driver[i]->rasterNumLevels = null::rasterNumLevels;
		engine->driver[i]->rasterFromImage = null::rasterFromImage;
		engine->driver[i]->rasterToImage = null::rasterToImage;
	}

	Engine::state = Opened;
	return 1;
}

// This is where RW creates the actual rendering device
// ans calls the engine plugin ctors
bool32
Engine::start(EngineStartParams *p)
{
	if(engine == nil || Engine::state != Opened){
		RWERROR((ERR_ENGINESTART));
		return 0;
	}

	// Start device
	engine->device.system(DEVICESTART, (void*)p);
	engine->device.system(DEVICEINIT, nil);

	// TODO: construct engine plugins
	Frame::dirtyList.init();
	for(uint i = 0; i < NUM_PLATFORMS; i++)
		Driver::s_plglist[i].construct(rw::engine->driver[i]);

	// TODO: finalize device start

	Engine::state = Started;
	return 1;
}

void
Engine::term(void)
{
	// TODO
}

void
Engine::close(void)
{
	// TODO
	rwFree(engine);
}

void
Engine::stop(void)
{
	engine->device.system(DEVICESTOP, nil);
}

namespace null {

void beginUpdate(Camera*) { }
void endUpdate(Camera*) { }
void clearCamera(Camera*,RGBA*,uint32) { }
void showRaster(Raster*) { }

void   setRenderState(int32, uint32) { }
uint32 getRenderState(int32) { return 0; }

void im2DRenderIndexedPrimitive(PrimitiveType, void*, int32, void*, int32) { }

void
rasterCreate(Raster*)
{
	assert(0 && "rasterCreate not implemented");
}

uint8*
rasterLock(Raster*, int32)
{
	assert(0 && "lockRaster not implemented");
	return nil;
}

void
rasterUnlock(Raster*, int32)
{
	assert(0 && "unlockRaster not implemented");
}

int32
rasterNumLevels(Raster*)
{
	assert(0 && "rasterNumLevels not implemented");
	return 0;
}

void
rasterFromImage(Raster*, Image*)
{
	assert(0 && "rasterFromImage not implemented");
}

Image*
rasterToImage(Raster*)
{
	assert(0 && "rasterToImage not implemented");
	return nil;
}

int
deviceSystem(DeviceReq req, void *arg0)
{
	return 1;
}

Device renderdevice = {
	0.0f, 1.0f,
	null::beginUpdate,
	null::endUpdate,
	null::clearCamera,
	null::showRaster,
	null::setRenderState,
	null::getRenderState,
	null::im2DRenderIndexedPrimitive,
	null::deviceSystem
};

}
}
