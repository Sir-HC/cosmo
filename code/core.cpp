
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

inline tile_map* GetTileMap(world *World, int32 TileMapX, int32 TileMapY){
	tile_map *TileMap = 0;
	
	if((TileMapX >= 0) && (TileMapX < World->TileMapCountX) &&
	   (TileMapY >= 0) && (TileMapY < World->TileMapCountY)){
		TileMap = &World->TileMaps[TileMapY*World->TileMapCountX + TileMapX];
	}
	
	return(TileMap);
}

inline uint32 GetTileUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY){
	Assert(TileMap);
	Assert((TileX >=0) && (TileX < World->CountX) &&
		   (TileY >=0) && (TileY < World->CountY));
	uint32 Result = TileMap->Tiles[TileY*World->CountX + TileX];
	return(Result);
}

internal bool32 IsMapPointEmpty(world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY){
	bool32 IsEmpty = false;
	if(TileMap){
		if((TestTileX >=0) && (TestTileX < World->CountX) &&
		   (TestTileY >=0) && (TestTileY < World->CountY)){
			uint32 TileMapValue = GetTileUnchecked(World, TileMap, TestTileX, TestTileY);
			IsEmpty = (TileMapValue == 0);
		}
	}
	return IsEmpty;
}

internal canonical_position GetCanonicalPosition(world *World, raw_position Pos){
	canonical_position Result;
	
	Result.TileMapX = Pos.TileMapX;
	Result.TileMapY = Pos.TileMapY;
	
	real32 ScreenXinTileMap = Pos.X - World->UpperLeftX;
	real32 ScreenYinTileMap = Pos.Y - World->UpperLeftY;
	
	Result.TileX = FloorReal32ToInt32((ScreenXinTileMap)/World->TileSideInPixels);
	Result.TileY = FloorReal32ToInt32((ScreenYinTileMap)/World->TileSideInPixels);
	
	Result.X = ScreenXinTileMap - Result.TileX * World->TileSideInPixels;
	Result.Y = ScreenYinTileMap - Result.TileY * World->TileSideInPixels;
	
	if(Result.TileX < 0){
		Result.TileX = World->CountX + Result.TileX;
		--Result.TileMapX;
	}
	if(Result.TileY < 0){
		Result.TileY = World->CountY + Result.TileY;
		--Result.TileMapY;
	}
	if(Result.TileX >= World->CountX){
		Result.TileX = Result.TileX - World->CountX;
		++Result.TileMapX;
	}
	if(Result.TileY >= World->CountY){
		Result.TileY = Result.TileY - World->CountY;
		++Result.TileMapY;
	}
	return(Result);
	
}

internal bool32 IsWorldPointEmpty(world *World, raw_position Pos){
	bool32 IsEmpty = false;
	
	canonical_position CanPos = GetCanonicalPosition(World, Pos);
	
	tile_map *TileMap = GetTileMap(World, CanPos.TileMapX, CanPos.TileMapY);
	IsEmpty = IsMapPointEmpty(World, TileMap, CanPos.TileX, CanPos.TileY);
	
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


#define TILE_MAP_COUNT_X 17
#define TILE_MAP_COUNT_Y 9
	uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
	};
	uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
	};
	uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 1, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0},
		{1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
	};
	uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1,  1, 1, 1, 1,  0, 1, 1, 1,  1, 1, 1, 1,  1},
		{1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  1, 0, 1, 0,  0, 0, 0, 0,  0, 1, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 1, 0,  0, 1, 0, 0,  0, 0, 0, 0,  1},
		{1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1,  1},
		{1, 1, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  1},
														  
		{1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1, 1, 1, 1,  1},
	};
	
	tile_map TileMaps[2][2]; 
	TileMaps[0][0].Tiles = (uint32 *)Tiles00;
	TileMaps[0][1].Tiles = (uint32 *)Tiles01;
	TileMaps[1][0].Tiles = (uint32 *)Tiles10;
	TileMaps[1][1].Tiles = (uint32 *)Tiles11;
	
	world World; 
	World.CountX = TILE_MAP_COUNT_X;
	World.CountY = TILE_MAP_COUNT_Y;
	World.TileSideInMeters = 1.4f;
	World.TileSideInPixels = 60;
	World.UpperLeftX = -(real32)World.TileSideInPixels/2;
	World.UpperLeftY = 0;
	World.TileMapCountX = 2;
	World.TileMapCountY = 2;
	real32 PlayerWidth = 0.75f*World.TileSideInPixels;
	real32 PlayerHeight = (real32)World.TileSideInPixels;
	
	World.TileMaps = (tile_map *)TileMaps;
	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	
	if(!Memory->IsInitilized){
		GameState->PlayerX = 100;
		GameState->PlayerY = 200;
		GameState->PlayerTileMapX = 0;
		GameState->PlayerTileMapY = 0;
		
		Memory->IsInitilized = true;
	}
	
	tile_map *TileMap = GetTileMap(&World, GameState->PlayerTileMapX, GameState->PlayerTileMapY);
	Assert(TileMap);
	
	for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->IsAnalog){
			
		} else {
			real32 dPlayerX = 0.0f;
			real32 dPlayerY = 0.0f;
			if (Controller->MoveUp.EndedDown){
				dPlayerY = -1.0f;
			}
			if (Controller->MoveDown.EndedDown){
				dPlayerY = 1.0f;
			}
			if (Controller->MoveLeft.EndedDown){
				dPlayerX = -1.0f;
			}
			if (Controller->MoveRight.EndedDown){
				dPlayerX = 1.0f;
			}
			real32 NewPlayerX = GameState->PlayerX + Input->dtForFrame * dPlayerX * 128;
			real32 NewPlayerY = GameState->PlayerY + Input->dtForFrame * dPlayerY * 128;
			
			raw_position PlayerPos = {
				GameState->PlayerTileMapX,
				GameState->PlayerTileMapY,
				NewPlayerX,
				NewPlayerY,
			};
			
			raw_position PlayerLeft = PlayerPos;
			PlayerLeft.X -= 0.5f*PlayerWidth;
			
			raw_position PlayerRight = PlayerPos;
			PlayerRight.X += 0.5f*PlayerWidth;
			
			raw_position PlayerTopHit = PlayerPos;
			PlayerTopHit.Y -= 0.25f*PlayerHeight;
			
			if(IsWorldPointEmpty(&World, PlayerPos) &&
			   IsWorldPointEmpty(&World, PlayerLeft) &&
			   IsWorldPointEmpty(&World, PlayerRight) &&
			   IsWorldPointEmpty(&World, PlayerTopHit)
			){
				canonical_position CanPos = GetCanonicalPosition(&World, PlayerPos);
				
				GameState->PlayerTileMapX = CanPos.TileMapX;
				GameState->PlayerTileMapY = CanPos.TileMapY;
				
				GameState->PlayerX = World.UpperLeftX + World.TileSideInPixels*CanPos.TileX + CanPos.X;
				GameState->PlayerY = World.UpperLeftY + World.TileSideInPixels*CanPos.TileY + CanPos.Y;
				
			}
		}
	}
	
	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 1.0f);
	
	for(int Row = 0; Row < TILE_MAP_COUNT_Y; ++Row){
		for(int Col = 0; Col < TILE_MAP_COUNT_X; ++Col){
			uint32 TileID = GetTileUnchecked(&World, TileMap, Col, Row);
			real32 Grey = 0.3f;
			if(TileID == 1){
				Grey = 0.9f;
			}
			
			real32 MinX = World.UpperLeftX + ((real32)Col) * World.TileSideInPixels;
			real32 MinY = World.UpperLeftY + ((real32)Row) * World.TileSideInPixels;
			real32 MaxX = MinX + World.TileSideInPixels;
			real32 MaxY = MinY + World.TileSideInPixels;
			DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Grey, Grey, Grey);
		}
	}
	
	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	
	real32 PlayerLeft = GameState->PlayerX - 0.5f*PlayerWidth;
	real32 PlayerTop = GameState->PlayerY - PlayerHeight;
	DrawRectangle(Buffer, PlayerLeft, PlayerTop, PlayerLeft+PlayerWidth, PlayerTop+PlayerHeight, PlayerR, PlayerG, PlayerB);
	
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples){
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

