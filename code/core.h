#if !defined(CORE)

//PERFORMANCE_SLOW:
// 		1 - Slow code
//      0 - No slow code
//CORE_INTERNAL:
//      1 - Developer build
//      0 - Shipping build


#include "core_platform.h"


#include "core_math.h"
#include "lynch_intrinsics.h"
#include "lynch_tile.h"

struct memory_arena{
	memory_index Size;
	uint8 *Base;
	memory_index Used;
};

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))

void *
PushSize_(memory_arena *Arena, memory_index Size){
	Assert((Arena->Used + Size) <= Arena->Size);
	void *Result = Arena->Base + Arena->Used;
	Arena->Used += Size;
	return(Result);
}

struct world{
	tile_map *TileMap;
};

struct loaded_bitmap{
	int32 Width;
	int32 Height;
	uint32 *Pixels;
};

struct hero_bitmaps{
	int32 AlignX;
	int32 AlignY;
	loaded_bitmap Head;
	loaded_bitmap Cape;
	loaded_bitmap Torso;
	
};

struct game_state{
	memory_arena WorldArena;
	world *World;
	tile_map_position PlayerPos;
	tile_map_position CameraPos;
	v2 dPlayerPos;
	
	
	loaded_bitmap Backdrop;
	uint32 HeroFacingDirection;
	hero_bitmaps HeroBitmaps[4];
};

#define CORE
#endif