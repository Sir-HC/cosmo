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

struct tile_map {
	
	uint32 *Tiles;
};

struct canonical_position {

	int32 TileMapX;
	int32 TileMapY; 
	
	int32 TileX;
	int32 TileY;
	
	// x/y from tile
	real32 X;
	real32 Y;
};

struct raw_position {
	int32 TileMapX;
	int32 TileMapY;
	
	// x/y from map
	real32 X;
	real32 Y;
};

struct world{
	
	real32 TileSideInMeters;
	int32 TileSideInPixels; 
	int32 CountX;
	int32 CountY;
	real32 UpperLeftX;
	real32 UpperLeftY;
	
	int32 TileMapCountX;
	int32 TileMapCountY;
	
	tile_map *TileMaps;
};



struct game_state{
	int32 PlayerTileMapX;
	int32 PlayerTileMapY;
	
	real32 PlayerX;
	real32 PlayerY;
	
};

#define CORE
#endif