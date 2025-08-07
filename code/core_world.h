#if !defined(LYNCH_TILE_H)

struct world_difference{
	v2 dXY;
	real32 dZ;
	
};
struct world_position {
	
	int32 ChunkX;
	int32 ChunkY;
	int32 ChunkZ;
	
	// From Chunk center
	v2 Offset_;
};

struct world_entity_block{
	uint32 EntityCount;
	uint32 ExternalEntityIndex[16];
	world_entity_block *Next;
};

struct world_chunk {
	int32 ChunkX;
	int32 ChunkY;
	int32 ChunkZ;
	
	world_entity_block FirstBlock;
	
	world_chunk *NextInHash;
};

struct world{
	real32 TileSideInMeters;
	real32 ChunkSideInMeters;
	
	world_chunk ChunkHash[4096];
	
	world_entity_block *FirstFree;
};

#define LYNCH_TILE_H
#endif 