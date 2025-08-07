#define CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define CHUNK_UNINITALIZED INT32_MAX
#define TILES_PER_CHUNK 16

inline bool32 IsCanonical(world *World, real32 TileRel){
	bool32 Res = ((TileRel >= -0.5f * World->ChunkSideInMeters) && (TileRel <= 0.5f * World->ChunkSideInMeters));
	return Res;
}

inline bool32 IsCanonical(world *World, v2 Offset){
	bool32 Res = (IsCanonical(World, Offset.X) && IsCanonical(World, Offset.Y));
	return Res;
}

internal bool32 AreInSameChunk(world *World, world_position *PosA, world_position *PosB){
	Assert(IsCanonical(World, PosA->Offset_));
	Assert(IsCanonical(World, PosB->Offset_));
	bool32 Res = (PosA->ChunkX == PosB->ChunkX) &&
				 (PosA->ChunkY == PosB->ChunkY) &&
				 (PosA->ChunkZ == PosB->ChunkZ);
	return(Res);
} 


inline world_chunk* GetWorldChunk(world *World, int32 ChunkX, int32 ChunkY, int32 ChunkZ, memory_arena *Arena = 0 ){
	Assert(ChunkX > -CHUNK_SAFE_MARGIN);
	Assert(ChunkY > -CHUNK_SAFE_MARGIN); 
	Assert(ChunkZ > -CHUNK_SAFE_MARGIN);
	Assert(ChunkX < CHUNK_SAFE_MARGIN);
	Assert(ChunkY < CHUNK_SAFE_MARGIN); 
	Assert(ChunkZ < CHUNK_SAFE_MARGIN);
	
	uint32 HashVal = 19*ChunkX + 7*ChunkY + 3*ChunkZ;
	uint32 HashSlot = HashVal & (ArrayCount(World->ChunkHash) - 1);
	Assert(HashSlot < ArrayCount(World->ChunkHash));
	world_chunk *Chunk = World->ChunkHash + HashSlot;
	
	//if arena not provided will return 0 on empty/invalid
	do {
		if((ChunkX == Chunk->ChunkX) && 
		   (ChunkY == Chunk->ChunkY) && 
		   (ChunkZ == Chunk->ChunkZ)){
				break;
			}
		// Hash occupied, push next and advance
		if(Arena && (Chunk->ChunkX != CHUNK_UNINITALIZED) && (!Chunk->NextInHash)){
			Chunk->NextInHash = PushStruct(Arena, world_chunk);
			Chunk->NextInHash->ChunkX = CHUNK_UNINITALIZED;
			Chunk = Chunk->NextInHash;
		}
		//Hash not occupied, put data, set next as 0
		if(Arena && (Chunk->ChunkX == CHUNK_UNINITALIZED)){
			
			Chunk->ChunkX = ChunkX;
			Chunk->ChunkY = ChunkY;
			Chunk->ChunkZ = ChunkZ;
			
			Chunk->NextInHash = 0;
			break;
		}
		Chunk = Chunk->NextInHash;
	} while(Chunk);
	
	return(Chunk);
}


inline world_position ChunkPositionFromTilePosition(world *World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ){
	world_position Res = {};
	Res.ChunkX = AbsTileX / TILES_PER_CHUNK;
	Res.ChunkY = AbsTileY / TILES_PER_CHUNK;
	Res.ChunkZ = AbsTileZ / TILES_PER_CHUNK;
	
	Res.Offset_.X = (real32)(AbsTileX - (Res.ChunkX * TILES_PER_CHUNK)) * World->TileSideInMeters;
	Res.Offset_.Y = (real32)(AbsTileY - (Res.ChunkY * TILES_PER_CHUNK)) * World->TileSideInMeters;
	
	return(Res);
}


inline void RecanonicalizeCoord(world *World, int32 *Tile, real32 *Rel){
	int32 Offset = RoundReal32ToInt32(*Rel / World->ChunkSideInMeters);
	
	*Tile += Offset;
	*Rel -= Offset*World->ChunkSideInMeters;
	Assert(IsCanonical(World, *Rel));
}

inline world_position MapIntoChunkSpace(world *World, world_position BasePos, v2 Offset){
	
	world_position Res = BasePos;
	
	Res.Offset_ += Offset;
	RecanonicalizeCoord(World, &Res.ChunkX, &Res.Offset_.X);
	RecanonicalizeCoord(World, &Res.ChunkY, &Res.Offset_.Y);
	return(Res);
}

inline world_position RecanonicalizePosition(world *World, world_position Pos){
	world_position Result = Pos;
	
	RecanonicalizeCoord(World, &Result.ChunkX, &Result.Offset_.X);
	RecanonicalizeCoord(World, &Result.ChunkY, &Result.Offset_.Y);
	
	return(Result);
}

internal void InitWorldMap(world *World, real32 TileSideInMeters){
	World->TileSideInMeters = TileSideInMeters;
	World->ChunkSideInMeters = (real32)TILES_PER_CHUNK*TileSideInMeters;
	World->FirstFree = 0;
	for(uint32 ChunkIndex = 0; ChunkIndex < ArrayCount(World->ChunkHash); ++ChunkIndex){
		World->ChunkHash[ChunkIndex].ChunkX = CHUNK_UNINITALIZED;
		World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
	}
}

inline world_difference Subtract(world *World, world_position *A, world_position *B){
	world_difference Res;
	v2 dTileXY = {(real32)A->ChunkX - (real32)B->ChunkX, 
				  (real32)A->ChunkY - (real32)B->ChunkY};
				
	real32 dTileZ = (real32)A->ChunkZ - (real32)B->ChunkZ;
	
	Res.dXY = World->ChunkSideInMeters*dTileXY + (A->Offset_ - B->Offset_);
	Res.dZ = World->ChunkSideInMeters*dTileZ;
	return(Res);
}

inline world_position CenteredChunkPoint(uint32 ChunkX, uint32 ChunkY, uint32 ChunkZ){
	world_position Res = {};
	Res.ChunkX = ChunkX;
	Res.ChunkY = ChunkY;
	Res.ChunkZ = ChunkZ;
	return(Res);
}

// load or change entity pos
inline void ChangeEntityLocation(memory_arena *Arena, world *World, uint32 ExternalEntityIndex, world_position *OldPos, world_position *NewPos){
	if(OldPos && AreInSameChunk(World, OldPos, NewPos)){
			
	} else {
		if(OldPos){
			//pull out old pos (if not it didnt exist)
			world_chunk *Chunk = GetWorldChunk(World, OldPos->ChunkX, OldPos->ChunkY, OldPos->ChunkZ);
			Assert(Chunk);
			if(Chunk){
				bool32 NotFound = true;
				world_entity_block *FirstBlock = &Chunk->FirstBlock;
				for(world_entity_block *Block = FirstBlock; Block && NotFound; Block = Block->Next){
					for(uint32 EntityIndex = 0; (EntityIndex < Block->EntityCount) && NotFound; ++EntityIndex){
						if(Block->ExternalEntityIndex[EntityIndex] == ExternalEntityIndex){
							Block->ExternalEntityIndex[EntityIndex] = FirstBlock->ExternalEntityIndex[--FirstBlock->EntityCount];
							if(FirstBlock->EntityCount == 0){
								if(FirstBlock->Next){
									world_entity_block *NextBlock = FirstBlock->Next;
									*FirstBlock = *NextBlock;
									NextBlock->Next = World->FirstFree;
									World->FirstFree = NextBlock;
								}
							}
							NotFound = false;
						}
					}
				}
			}
		}
		//transfer entity to new chunk
		world_chunk *Chunk = GetWorldChunk(World, NewPos->ChunkX, NewPos->ChunkY, NewPos->ChunkZ, Arena);
		world_entity_block *Block = &Chunk->FirstBlock;
		if(Block->EntityCount == ArrayCount(Block->ExternalEntityIndex)){
			// make new block
			world_entity_block *OldBlock = World->FirstFree;
			if(OldBlock){
				World->FirstFree = OldBlock->Next; 
			} else{
				OldBlock = PushStruct(Arena, world_entity_block);
			}
			*OldBlock = *Block;
			Block->Next = OldBlock;
			Block->EntityCount = 0;
		}
		Assert(Block->EntityCount < ArrayCount(Block->ExternalEntityIndex));
		Block->ExternalEntityIndex[Block->EntityCount++] = ExternalEntityIndex;
	}
}






/*
inline tile_chunk_position GetChunkPositionFor(tile_map* TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ){
	tile_chunk_position Result;
	
	Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
	Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
	Result.TileChunkZ = AbsTileZ;
	Result.RelTileX = AbsTileX & TileMap->ChunkMask;
	Result.RelTileY = AbsTileY & TileMap->ChunkMask;
	
	return(Result);
}



inline uint32 GetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, int32 TileX, int32 TileY){
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);
	uint32 Result = TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX];
	return(Result);
}

inline void SetTileValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, int32 TileX, int32 TileY, uint32 TileValue){
	Assert(TileChunk);
	Assert(TileX < TileMap->ChunkDim);
	Assert(TileY < TileMap->ChunkDim);
	TileChunk->Tiles[TileY*TileMap->ChunkDim + TileX] = TileValue;
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
	tile_chunk *Chunk = GetWorldChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
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



internal bool32 IsTileMapPointEmpty(tile_map *TileMap, tile_map_position Pos){
	uint32 TileValue = GetTileValue(TileMap, Pos.AbsTileX, Pos.AbsTileY, Pos.AbsTileZ);
	bool32 IsEmpty = IsTileValueEmpty(TileValue);
	return IsEmpty;
}

inline void
SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue){
	tile_chunk_position ChunkPos = GetChunkPositionFor(TileMap, AbsTileX, AbsTileY, AbsTileZ);
	tile_chunk *Chunk = GetWorldChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ, Arena);
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
*/
