inline void RecanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *Rel){
	
	int32 Offset = RoundReal32ToInt32(*Rel / TileMap->TileSideInMeters);
	
	*Tile += Offset;
	*Rel -= Offset*TileMap->TileSideInMeters;
	
	Assert(*Rel >= -0.5f * TileMap->TileSideInMeters);
	Assert(*Rel <= 0.5f * TileMap->TileSideInMeters);
}

inline tile_map_position RecanonicalizePosition(tile_map *TileMap, tile_map_position Pos){
	tile_map_position Result = Pos;
	
	RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.Offset.X);
	RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Offset.Y);
	
	return(Result);
}

inline tile_chunk* GetTileChunk(tile_map *TileMap, uint32 TileChunkX, uint32 TileChunkY, uint32 TileChunkZ){
	tile_chunk *TileChunk = 0;
	
	if((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY) &&
	   (TileChunkZ >= 0) && (TileChunkZ < TileMap->TileChunkCountZ)){
		TileChunk = &TileMap->TileChunks[ TileChunkZ * TileChunkY*TileMap->ChunkDim +
										  TileChunkY * TileMap->ChunkDim + 
										  TileChunkX];
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

inline tile_chunk_position GetChunkPositionFor(tile_map* TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ){
	tile_chunk_position Result;
	
	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.TileChunkZ = AbsTileZ;
	Result.RelTileX = AbsTileX & TileMap->ChunkMask;
	Result.RelTileY = AbsTileY & TileMap->ChunkMask;
	
	return(Result);
}


internal void SetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
    if(TileChunk && TileChunk->Tiles)
    {
        SetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
    
}

internal uint32 GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;
    
    if(TileChunk && TileChunk->Tiles)
    {
        TileChunkValue = GetTileValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    
    return(TileChunkValue);
}

internal uint32 GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ){
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
	tile_chunk *Chunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
	uint32 TileChunkValue = GetTileValue(TileMap, Chunk, ChunkPos.RelTileX, ChunkPos.RelTileY);

	return TileChunkValue;
}

internal uint32 GetTileValue(tile_map *TileMap, tile_map_position Pos)
{
    uint32 TileVal = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
    
    return(TileVal);
}

internal bool32 IsTileValueEmpty(uint32 TileValue){
	bool32 Empty =  (TileValue == 1) || (TileValue == 3) || (TileValue == 4);
	return(Empty);
}

internal bool32 AreOnSameTile(tile_map_position *PosA, tile_map_position *PosB){
	bool32 Res = (PosA->AbsTileX == PosB->AbsTileX) &&
				 (PosA->AbsTileY == PosB->AbsTileY) &&
				 (PosA->AbsTileZ == PosB->AbsTileZ);
	return(Res);
}

internal bool32 IsTileMapPointEmpty(tile_map *TileMap, tile_map_position Pos){
	uint32 TileValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
	bool32 IsEmpty = IsTileValueEmpty(TileValue);
	return IsEmpty;
}

inline void
SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue){
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
	tile_chunk *Chunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
	// todo ondemand tile chunk create
	Assert(Chunk);
	if(!Chunk->Tiles){
		uint32 TileCount = TileMap->ChunkDim*TileMap->ChunkDim;
		Chunk->Tiles = PushArray(Arena, TileCount , uint32);
		for(uint32 TileIndex = 0; TileIndex < TileCount; ++TileIndex){
			Chunk->Tiles[TileIndex] = 1;
		}
	}
	SetTileValue(TileMap, Chunk, ChunkPos.RelTileX, ChunkPos.RelTileY, TileValue);
}

inline tile_map_difference Subtract(tile_map *TileMap, tile_map_position *A, tile_map_position *B){
	tile_map_difference Res;
	v2 dTileXY = {(real32)A->AbsTileX - (real32)B->AbsTileX, 
				  (real32)A->AbsTileY - (real32)B->AbsTileY};
				
	real32 dTileZ = (real32)A->AbsTileZ - (real32)B->AbsTileZ;
	
	Res.dXY = TileMap->TileSideInMeters*dTileXY + (A->Offset - B->Offset);
	Res.dZ = TileMap->TileSideInMeters*dTileZ;
	return(Res);
}

inline tile_map_position CenteredTilePoint(uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ){
	tile_map_position Res = {};
	Res.AbsTileX = AbsTileX;
	Res.AbsTileY = AbsTileY;
	Res.AbsTileZ = AbsTileZ;
	return(Res);
}
