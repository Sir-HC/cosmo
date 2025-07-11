inline void RecanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *Rel){
	
	int32 Offset = RoundReal32ToInt32(*Rel / TileMap->TileSideInMeters);
	
	*Tile += Offset;
	*Rel -= Offset*TileMap->TileSideInMeters;
	
	Assert(*Rel >= -0.5f * TileMap->TileSideInMeters);
	Assert(*Rel <= 0.5f * TileMap->TileSideInMeters);
}

inline tile_map_position RecanonicalizePosition(tile_map *TileMap, tile_map_position Pos){
	tile_map_position Result = Pos;
	
	RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.X);
	RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Y);
	
	return(Result);
}

inline tile_chunk* GetTileChunk(tile_map *TileMap, uint32 TileChunkX, uint32 TileChunkY){
	tile_chunk *TileChunk = 0;
	
	if((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY)){
		TileChunk = &TileMap->TileChunks[TileChunkY*TileMap->ChunkDim + TileChunkX];
	}
	
	return(TileChunk);
}

inline uint32 GetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32 TileX, uint32 TileY){
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);
	uint32 Result = TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX];
	return(Result);
}

inline void SetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32 TileX, uint32 TileY, uint32 TileValue){
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);
	TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX] = TileValue;
}

inline tile_chunk_position GetChunkPositionFor(tile_map* TileMap, uint32 AbsTileX, uint32 AbsTileY){
	tile_chunk_position Result;
	
	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.RelTileX = AbsTileX & TileMap->ChunkMask;
	Result.RelTileY = AbsTileY & TileMap->ChunkMask;
	
	return(Result);
}

internal uint32 GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;
    
    if(TileChunk)
    {
        TileChunkValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    
    return(TileChunkValue);
}

internal void SetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
    uint32 TileChunkValue = 0;
    
    if(TileChunk)
    {
        SetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
    
}


internal uint32 GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY){
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY);
	tile_chunk *Chunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
	uint32 TileChunkValue = GetTileValue(TileMap, Chunk, ChunkPos.RelTileX, ChunkPos.RelTileY);

	return TileChunkValue;
}

internal bool32 IsTileMapPointEmpty(tile_map *TileMap, tile_map_position Pos){
	uint32 TileValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY);
	bool32 IsEmpty = (TileValue == 0);
	return IsEmpty;
}

inline void
SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 TileValue){
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY);
	tile_chunk *Chunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
	// todo ondemand tile chunk create
	Assert(Chunk);
	
	SetTileValue(TileMap, Chunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}
