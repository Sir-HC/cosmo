#if !defined(LYNCH_TILE_H)
struct tile_chunk {
	
	uint32 *Tiles;
};

struct tile_chunk_position {
	uint32 TileChunkX;
	uint32 TileChunkY;
	
	uint32 RelTileX;
	uint32 RelTileY;
};

struct tile_map_position {
	
	uint32 AbsTileX;
	uint32 AbsTileY;
	
	// x/y from tile
	real32 X;
	real32 Y;
};

struct tile_map{
	uint32 ChunkShift;
	uint32 ChunkMask;
	uint32 ChunkDim;
	
	real32 TileSideInMeters;
	int32 TileSideInPixels; 
	real32 MetersToPixels;
	
	
	uint32 TileChunkCountX;
	uint32 TileChunkCountY;
	
	tile_chunk *TileChunks;
};

#define LYNCH_TILE_H
#endif