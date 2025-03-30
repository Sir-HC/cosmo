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

struct tile_chunk {
	
	uint32 *Tiles;
};

struct tile_chunk_position {
	uint32 TileChunkX;
	uint32 TileChunkY;
	
	uint32 RelTileX;
	uint32 RelTileY;
};

struct world_position {
	
	uint32 AbsTileX;
	uint32 AbsTileY;
	
	// x/y from tile
	real32 X;
	real32 Y;
};

struct world{
	
	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDim;
	
	real32 TileSideInMeters;
	int32 TileSideInPixels; 
	real32 MetersToPixels;
	
	
	int32 TileChunkCountX;
	int32 TileChunkCountY;
	
	tile_chunk *TileChunks;
};

struct game_state{
	world_position PlayerPos;
	
};

#define CORE
#endif