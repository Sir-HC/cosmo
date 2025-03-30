
#include "core.h"
#include "lynch_intrinsics.h"

internal void 
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	int16 ToneVolume = 600;
	int16* SampleOut = SoundBuffer->Samples;
	
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

	bool32 SoundType = 0;
	for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
		
		int16 SampleValue = 0;
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
		/*
		if (SoundType == 1){
			real32 SineValue = sinf(GameState->tSine);
			int16 SampleValue = (int16)(SineValue * ToneVolume);
			//int16 SampleValue = 0;
			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;
			
			GameState->tSine += (real32)(2.0f * Pi32 * 1.0f) / (real32)WavePeriod;
			if(GameState->tSine > 2.0f*Pi32){
				GameState->tSine -= (real32)(2.0f*Pi32);
			}
		} else if (SoundType == 2) {
			//Square
			int16 SampleValue = 0;
			if (SquareCount < WavePeriod / 2){
				SampleValue = (int16)(ToneVolume/4);
				//SampleValue = 0;
			} else if (SquareCount > WavePeriod / 2) {
				
				SampleValue = -1*ToneVolume;
			}
			if (SquareCount >= WavePeriod){
				SquareCount = 0;
			}
			SquareCount++;
			*SampleOut++ = SampleValue;
			*SampleOut++ = SampleValue;
			GameState->tSquare = SquareCount;
		} else if (SoundType == 3) {
			int16 SampleValue = 0;
			
			if (GameState->tDirection){
				
				SampleValue = GameState->tSquare;
			} else if(!GameState->tDirection){
				SampleValue = --GameState->tSquare;
			}
			if (SampleValue >= abs(ToneVolume)) {
				GameState->tDirection = !GameState->tDirection;
			}
		}
		*/
	}
}
	
/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{	
	uint8 *Row = (uint8 *)Buffer->Memory;
	
	for(int Y = 0;
		Y < Buffer->Height;
		++Y){
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0;
			X < Buffer->Width;
			++X){
			uint8 Blue = (uint8)(X+BlueOffset);
			uint8 Green = (uint8)(Y+GreenOffset);
			uint8 Red = (uint8)(X+Y);
			*Pixel++ = (((Red << 8) | Green << 8) | Blue);
		}
		Row += Buffer->Pitch;
	}
}
*/



internal void
DrawRectangle(game_offscreen_buffer *Buffer, 
			  real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY, 
			  real32 R, real32 G, real32 B){
	int32 MinX = RoundReal32ToInt32(RealMinX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MaxY = RoundReal32ToInt32(RealMaxY);
	
	if(MinX < 0){
		MinX = 0;
	}
	if(MinY < 0){
		MinY = 0;
	}
	
	if(MaxX > Buffer->Width){
		MaxX = Buffer->Width;
	}
	if(MaxY > Buffer->Height){
		MaxY = Buffer->Height;
	}
	
	uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
	uint32 Color = ((RoundReal32ToUInt32(R*255.0f) << 16) |
					(RoundReal32ToUInt32(G*255.0f) << 8) |
				    (RoundReal32ToUInt32(B*255.0f) << 0));
	
	uint8 *Row = ((uint8 *)Buffer->Memory + MinX*Buffer->BytesPerPixel + MinY*Buffer->Pitch);
	
	for(int Y = MinY; Y < MaxY; ++Y){
		uint32 *Pixel = (uint32 *)Row;
		for(int X = MinX; X < MaxX; ++X){
			
			*(uint32 *)Pixel++ = Color;
			
		}
		Row += Buffer->Pitch;
	}
}

internal void
RenderPlayer(game_offscreen_buffer *Buffer, int PlayerX, int PlayerY){
	uint8 *EndOfBuffer = (uint8 *)Buffer->Memory + Buffer->Pitch*Buffer->Height;
	uint32 Color = 0xFFFFFFFF;
	int Top = PlayerY;
	int Bottom = PlayerY+10;
	
	for(int X = PlayerX; X < PlayerX+10; ++X){
		uint8 *Pixel = ((uint8 *)Buffer->Memory + X*Buffer->BytesPerPixel + Top*Buffer->Pitch);
		for(int Y = Top; Y < Bottom; ++Y){
			if((Pixel >= Buffer->Memory) && (Pixel < EndOfBuffer)){
				*(uint32 *)Pixel = Color;
			}
			Pixel += Buffer->Pitch;
		}
	}
}

inline tile_chunk* GetTileChunk(world *World, int32 TileChunkX, int32 TileChunkY){
	tile_chunk *TileChunk = 0;
	
	if((TileChunkX >= 0) && (TileChunkX < World->TileChunkCountX) &&
	   (TileChunkY >= 0) && (TileChunkY < World->TileChunkCountY)){
		TileChunk = &World->TileChunks[TileChunkY*World->ChunkDim + TileChunkX];
	}
	
	return(TileChunk);
}

inline uint32 GetTileValueUnchecked(world *World, tile_chunk *TileChunk, uint32 TileX, uint32 TileY){
	Assert(TileChunk);
	Assert(TileX < World->ChunkDim);
	Assert(TileY < World->ChunkDim);
	uint32 Result = TileChunk->Tiles[TileY*World->ChunkDim + TileX];
	return(Result);
}

inline void RecanonicalizeCoord(world *World, uint32 *Tile, real32 *Rel){
	
	int32 Offset = FloorReal32ToInt32(*Rel / World->TileSideInMeters);
	
	*Tile += Offset;
	*Rel -= Offset*World->TileSideInMeters;
	
	Assert(*Tile >=0);
	Assert(*Rel <= World->TileSideInMeters);
}

inline world_position RecanonicalizePosition(world *World, world_position Pos){
	world_position Result = Pos;
	
	RecanonicalizeCoord(World, &Result.AbsTileX, &Result.X);
	RecanonicalizeCoord(World, &Result.AbsTileY, &Result.Y);
	
	return(Result);
}

inline tile_chunk_position GetChunkPositionFor(world* World, uint32 AbsTileX, uint32 AbsTileY){
	tile_chunk_position Result;
	
	Result.TileChunkX = AbsTileX >> World->ChunkShift;
	Result.TileChunkY = AbsTileY >> World->ChunkShift;
	Result.RelTileX = AbsTileX & World->ChunkMask;
	Result.RelTileY = AbsTileY & World->ChunkMask;
	
	return(Result);
}

inline uint32 GetTileValue(world *World, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;
    
    if(TileChunk)
    {
        TileChunkValue = GetTileValueUnchecked(World, TileChunk, TestTileX, TestTileY);
    }
    
    return(TileChunkValue);
}

inline uint32 GetTileValue(world *World, uint32 AbsTileX, uint32 AbsTileY){
	uint32 TileChunkValue = 0;
	tile_chunk_position ChunkPos = GetChunkPositionFor(World, AbsTileX, AbsTileY);
	tile_chunk *Chunk = GetTileChunk(World, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
	TileChunkValue = GetTileValue(World, Chunk, ChunkPos.RelTileX, ChunkPos.RelTileY);

	return TileChunkValue;
}

internal bool32 IsWorldPointEmpty(world *World, world_position Pos){
	uint32 TileValue = GetTileValue(World, Pos.AbsTileX, Pos.AbsTileY);
	bool32 IsEmpty = (TileValue == 0);
	return IsEmpty;
}

/*
internal bool32 IsWorldPointEmpty(world *World, int32 TileMapX, int32 TileMapY, real32 TestX, real32 TestY){
	bool32 IsEmpty = false;
	
	tile_map *TileMap = GetTileMap(World, TileMapX, TileMapY);
	
	if(TileMap){
		uint32 TestTileX = TruncateReal32ToInt32((TestX - TileMap->UpperLeftX)/TileMap->TileWidth);
		uint32 TestTileY = TruncateReal32ToInt32((TestY - TileMap->UpperLeftY)/TileMap->TileHeight);
		if((TestTileX >=0) && (TestTileX < TileMap->CountX) &&
		   (TestTileY >=0) && (TestTileY < TileMap->CountY)){
			uint32 TileMapValue = GetTileUnchecked(TileMap, TestTileX, TestTileY);
			IsEmpty = (TileMapValue == 0);
		}
	}
	return IsEmpty;
}
*/

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender){
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));


#define TILE_MAP_COUNT_X 256
#define TILE_MAP_COUNT_Y 256
	uint32 TempTiles[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 0, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},						  												  
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},				  											  
		{1, 1, 1, 1, 1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 1, 1, 1, 1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 1, 0, 1, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1, 1, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},					  
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1, 1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1, 1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0, 0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1, 1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},										  
		{1, 1, 1, 1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
	};
	
	world World;
	World.ChunkShift = 8;
	World.ChunkMask = (1 << World.ChunkShift) - 1;
	World.TileChunkCountX = 1;
	World.TileChunkCountY = 1;
	
	tile_chunk TileChunk;
	TileChunk.Tiles = (uint32 *)TempTiles;
	World.TileChunks = &TileChunk;
	
	World.ChunkDim = 256;
	
	World.TileSideInMeters = 1.4f;
	World.TileSideInPixels = 60;
	World.MetersToPixels = (real32)World.TileSideInPixels / (real32)World.TileSideInMeters;
	
	real32 LowerLeftX = -(real32)World.TileSideInPixels/2;
	real32 LowerLeftY = (real32)Buffer->Height;
	
	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f*PlayerHeight;
	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	
	if(!Memory->IsInitilized){
		GameState->PlayerPos.AbsTileX = 3;
		GameState->PlayerPos.AbsTileY = 3;
		GameState->PlayerPos.X = 0.5f;
		GameState->PlayerPos.Y = 0.5f;
		
		Memory->IsInitilized = true;
	}
	
	for(int ControllerIndex = 0; ControllerIndex < 1; ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->IsAnalog){
			
		} else {
			real32 dPlayerX = 0.0f;
			real32 dPlayerY = 0.0f;
			if (Controller->MoveUp.EndedDown){
				dPlayerY = 1.0f;
			}
			if (Controller->MoveDown.EndedDown){
				dPlayerY = -1.0f;
			}
			if (Controller->MoveLeft.EndedDown){
				dPlayerX = -1.0f;
			}
			if (Controller->MoveRight.EndedDown){
				dPlayerX = 1.0f;
			}
			//meters/sec
			dPlayerX *= 5.0f;
			dPlayerY *= 5.0f;
			
			world_position NewPlayerPos = GameState->PlayerPos;
			NewPlayerPos.X += Input->dtForFrame * dPlayerX;
			NewPlayerPos.Y += Input->dtForFrame * dPlayerY;
			NewPlayerPos = RecanonicalizePosition(&World, NewPlayerPos);
			
			world_position PlayerLeft = NewPlayerPos;
			PlayerLeft.X -= 0.5f*PlayerWidth;
			PlayerLeft = RecanonicalizePosition(&World, PlayerLeft);
			
			world_position PlayerRight = NewPlayerPos;
			PlayerRight.X += 0.5f*PlayerWidth;
			PlayerRight = RecanonicalizePosition(&World, PlayerRight);
			
			world_position PlayerTopHit = NewPlayerPos;
			PlayerTopHit.Y += 0.25f*PlayerHeight;
			PlayerTopHit = RecanonicalizePosition(&World, PlayerTopHit);
			
			if(IsWorldPointEmpty(&World, NewPlayerPos) &&
			   IsWorldPointEmpty(&World, PlayerLeft) &&
			   IsWorldPointEmpty(&World, PlayerRight) &&
			   IsWorldPointEmpty(&World, PlayerTopHit)
			){
				GameState->PlayerPos = NewPlayerPos;
				
			}
		}
	}
	
	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 1.0f);
	
	real32 CenterX = 0.5f * (real32)Buffer->Width;
	real32 CenterY = 0.5f * (real32)Buffer->Height;
	
	for(int32 RelRow = -10; RelRow < 10; ++RelRow){
		for(int32 RelCol = -20; RelCol < 20; ++RelCol){
			uint32 Col = RelCol + GameState->PlayerPos.AbsTileX;
			uint32 Row = RelRow + GameState->PlayerPos.AbsTileY;
			uint32 TileID = GetTileValue(&World, Col, Row);
			real32 Grey = 0.3f;
			if(TileID == 1){
				Grey = 0.9f;
			}
			
			if((Row == GameState->PlayerPos.AbsTileY) && (Col == GameState->PlayerPos.AbsTileX)){
				Grey = 0.0f;
			}
			real32 MinX = CenterX + ((real32)RelCol) * World.TileSideInPixels;
			real32 MinY = CenterY - ((real32)RelRow) * World.TileSideInPixels;
			real32 MaxX = MinX + World.TileSideInPixels;
			real32 MaxY = MinY - World.TileSideInPixels;
			DrawRectangle(Buffer, MinX, MaxY, MaxX, MinY, Grey, Grey, Grey);
		}
	}
	
	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	
	real32 PlayerLeft = CenterX + World.MetersToPixels * GameState->PlayerPos.X -
						World.MetersToPixels * 0.5f * PlayerWidth;
	real32 PlayerTop =  CenterY - World.MetersToPixels * GameState->PlayerPos.Y -
						World.MetersToPixels * PlayerHeight;
	DrawRectangle(Buffer, PlayerLeft, PlayerTop, PlayerLeft+World.MetersToPixels*PlayerWidth, PlayerTop+World.MetersToPixels*PlayerHeight, PlayerR, PlayerG, PlayerB);
	
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples){
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

