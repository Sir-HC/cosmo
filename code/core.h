#if !defined(CORE)

//PERFORMANCE_SLOW:
// 		1 - Slow code
//      0 - No slow code
//CORE_INTERNAL:
//      1 - Developer build
//      0 - Shipping build


#include "core_platform.h"

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265


#if PERFORMANCE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif


#define ArrayCount(Array) (sizeof(Array)/ sizeof((Array)[0]))

#define Kilobytes(Value) (Value*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)


inline uint32
SafeTruncateUInt64(uint64 Value){
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return(Result);
}

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex){
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return(Result);
}



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

struct game_state{
	memory_arena WorldArena;
	world *World;
	tile_map_position PlayerPos;
	
	uint32 PixelWidth;
	uint32 PixelHeight;
	loaded_bitmap Backdrop;
	loaded_bitmap HeroHead;
	loaded_bitmap HeroCape;
	loaded_bitmap HeroTorso;
	
};

#define CORE
#endif