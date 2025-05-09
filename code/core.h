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

struct world{
	tile_map *TileMap;
};

struct game_state{
	tile_map_position PlayerPos;
	
};

#define CORE
#endif