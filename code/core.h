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
#include "core_world.h"

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

enum entity_type {
	EntityType_Null,
	EntityType_Player,
	EntityType_Wall,
};

struct local_entity{
	v2 Pos;
	v2 dPos;
	uint32 ChunkZ;
	uint32 FacingDirection;
	
	uint32 ExternalEntityIndex;
};

struct external_entity{
	entity_type Type;
	
	world_position Pos;
	real32 Width;
	real32 Height;
	
	bool32 Collidable;
	int32 dAbsTileZ;
	
	uint32 LocalEntityIndex;
};

enum entity_state{
	EntityState_Nonexistent,
	EntityState_External,
	EntityState_Local,
	
};

struct entity{
	uint32 ExternalIndex;
	local_entity *Local;
	external_entity *External;
	
};
#if 0
struct external_entity_chunk_reference{
	world_chunk *TileChunk;
	uint32 EntityIndexInChunk;
};
#endif
struct game_state{
	memory_arena WorldArena;
	world *World;
	
	uint32 CameraFollowingEntityIndex;
	world_position CameraPos;
	
	uint32 PlayerIndexForController[ArrayCount(((game_input *)0)->Controllers)];
	uint32 LocalEntityCount;
	local_entity LocalEntities[256];
	
	uint32 ExternalEntityCount;
	external_entity ExternalEntities[65536];
	
	loaded_bitmap Tree;
	loaded_bitmap Backdrop;
	hero_bitmaps HeroBitmaps[4];
};

#define CORE
#endif