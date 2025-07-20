
#include "core.h"
#include "core_random.h"
#include "lynch_tile.cpp"


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
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
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

#pragma pack(push, 1)
struct bitmap_header{
	uint16 FileType;
	uint32 FileSize;
	uint16 Reserved1;
	uint16 Reserved2;
	uint32 BitmapOffset;
	uint32 Size;
	int32 Width;
	int32 Height;
	uint16 Planes;
	uint16 BitsPerPixel;
	uint32 Compression;
	uint32 SizeOfBitmap;
	int32 HorzResolution;
	int32 VertResolution;
	uint32 ColorsUsed;
	uint32 ColorsImportant;
	
	uint32 RedMask;
	uint32 GreenMask;
	uint32 BlueMask;
};
#pragma pack(pop)



internal loaded_bitmap DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename){
	// Byte order AA BB GG RR
	loaded_bitmap Result = {};
	debug_read_file_result ReadRes = ReadEntireFile(Thread, Filename);
	if(ReadRes.ContentSize != 0){
		bitmap_header *Header = (bitmap_header *)ReadRes.Content;
		uint32 *Pixels = (uint32 *)((uint8 * )ReadRes.Content + Header->BitmapOffset);
		Result.Pixels = Pixels;
		Result.Height = Header->Height;
		Result.Width = Header->Width;
		
		Assert(Header->Compression == 3);
		
		int32 RedMask = Header->RedMask;
		int32 GreenMask = Header->GreenMask;
		int32 BlueMask = Header->BlueMask;
		int32 AlphaMask = ~(RedMask | GreenMask | BlueMask);
		
		bit_scan_result RedShift = FindLeastSigSetBit(RedMask);
		bit_scan_result GreenShift = FindLeastSigSetBit(GreenMask);
		bit_scan_result BlueShift = FindLeastSigSetBit(BlueMask);
		bit_scan_result AlphaShift = FindLeastSigSetBit(AlphaMask);
		
		Assert(RedShift.Found);
		Assert(GreenShift.Found);
		Assert(BlueShift.Found);
		Assert(AlphaShift.Found);
		
		uint32 *SourceDest = Pixels;
		for(int32 Y = 0; Y < Header->Height; ++Y){
			for(int32 X = 0; X < Header->Width; ++X){
				uint32 C = *SourceDest;
				*SourceDest++ = ((((C >> AlphaShift.Index) & 0xFF) << 24) | 
							   (((C >> RedShift.Index) & 0xFF) << 16) | 
							   (((C >> GreenShift.Index) & 0xFF) << 8) | 
							   (((C >> BlueShift.Index) & 0xFF) << 0));
			}
		}
	}
	
	return(Result);
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

internal void
InitArena(memory_arena *Arena, memory_index Size, uint8 *Base){
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}	


internal void DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY){
	
	int32 MinX = RoundReal32ToInt32(RealX);
	int32 MaxX = RoundReal32ToInt32(RealX+Bitmap->Width);
	int32 MinY = RoundReal32ToInt32(RealY);
	int32 MaxY = RoundReal32ToInt32(RealY+Bitmap->Height);
	
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
	

	uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height-1);
	uint8 *DestRow = ((uint8 *)Buffer->Memory + MinX*Buffer->BytesPerPixel + MinY*Buffer->Pitch);

	for(int Y = MinY; Y < MaxY; ++Y){
		uint32 *Dest = (uint32 *)DestRow;
		uint32 *Source = SourceRow;
		for(int X = MinX; X < MaxX; ++X){
			uint32 alpha = *Source >> 24;
			if(alpha == 0xFF){	
				*Dest = *Source;
			}else if(alpha < 0xFF && alpha > 0x00){
				real32 A =  (real32)((*Source >> 24) & 0xFF) / 255.0f;
				real32 SR = (real32)((*Source >> 16) & 0xFF);
				real32 SG = (real32)((*Source >> 8) & 0xFF);
				real32 SB = (real32)((*Source >> 0) & 0xFF);
				
				real32 DR = (real32)((*Dest >> 16) & 0xFF);
				real32 DG = (real32)((*Dest >> 8) & 0xFF);
				real32 DB = (real32)((*Dest >> 0) & 0xFF);
				
				real32 R = (1.0f - A) * DR + A * SR;
				real32 G = (1.0f - A) * DG + A * SG;
				real32 B = (1.0f - A) * DB + A * SB;
				
				*Dest = (uint32(R + 0.5f) << 16) | (uint32(G + 0.5f) << 8) | uint32(B + 0.5f);
				
			}
			 
			++Dest;
			++Source;
		}
		DestRow += Buffer->Pitch;
		SourceRow -= Bitmap->Width;;
	}
}


extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender){
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));


#define TILE_MAP_COUNT_X 256
#define TILE_MAP_COUNT_Y 256
	

	game_state *GameState = (game_state *)Memory->PermanentStorage;
	real32 PlayerHeight = 1.4f;
	real32 PlayerWidth = 0.75f*PlayerHeight;
	if(!Memory->IsInitilized){
		GameState->Backdrop = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_background.bmp");
		
		GameState->HeroHead = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_head.bmp");
		GameState->HeroCape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_cape.bmp");
		GameState->HeroTorso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_torso.bmp");
		
		GameState->PlayerPos.AbsTileX = 5;
		GameState->PlayerPos.AbsTileY = 2;
		GameState->PlayerPos.X = 5.5f;
		GameState->PlayerPos.Y = 5.5f;
		InitArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *)Memory->PermanentStorage + sizeof(game_state)); 
		GameState->World = PushStruct(&GameState->WorldArena, world);
		world *World = GameState->World;
		World->TileMap = PushStruct(&GameState->WorldArena, tile_map);
		
		tile_map *TileMap = World->TileMap;
		TileMap->ChunkShift = 4;
		TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
		TileMap->ChunkDim = (1 << TileMap->ChunkShift);
		TileMap->TileChunkCountX = 128;
		TileMap->TileChunkCountY = 128;
		TileMap->TileChunkCountZ = 4;
		
		TileMap->TileChunks = PushArray(&GameState->WorldArena, TileMap->TileChunkCountX *
																TileMap->TileChunkCountY *
																TileMap->TileChunkCountZ, tile_chunk);
		
		/*
		for(uint32 Y = 0; Y < TileMap->TileChunkCountY; ++Y){
			for(uint32 X = 0; X < TileMap->TileChunkCountX; ++X){
				uint32 TileChunkIndex = Y*TileMap->TileChunkCountX + X;
				uint32 TileCount = TileMap->ChunkDim*TileMap->ChunkDim;
				TileMap->TileChunks[TileChunkIndex].Tiles = PushArray(&GameState->WorldArena,TileCount , uint32);
				for(uint32 TileIndex = 0; TileIndex < TileCount; ++TileIndex){
					TileMap->TileChunks[TileChunkIndex].Tiles[TileIndex] = 1;
				}
			}
		}*/
		
		TileMap->TileSideInMeters = 1.4f;
		
	
		
		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;
		uint32 ScreenY = 0;
		uint32 ScreenX = 0;
		uint32 AbsTileZ = 0;
			
		bool32 DoorEast = false;
		bool32 DoorWest = false;
		bool32 DoorNorth = false;
		bool32 DoorSouth = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;
		
		uint32 RandomNumberIndex = 0;
		for(uint32 ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex){
			uint32 RandomChoice;
			if(DoorUp || DoorDown) {
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2 + 1;
			}else{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
			}
			bool32 ZDoorMade = false;
			if (RandomChoice == 0){
				ZDoorMade = true;
				if(AbsTileZ == 0){
					DoorUp = true;
				} else{
					DoorDown = true;
				}
			}
			else if(RandomChoice == 1){
				DoorEast = true;
			} else{
				DoorNorth = true;
			}
			
			for(uint32 TileY = 0; TileY < TilesPerHeight; ++TileY){
				for(uint32 TileX = 0; TileX < TilesPerWidth; ++TileX){
					uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
					uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;
					
					uint32 TileValue = 1;
					if((TileX == 0) && (!DoorWest || (TileY != (TilesPerHeight/2)))){
						TileValue = 2;
					}
					if((TileX == (TilesPerWidth - 1)) && (!DoorEast || (TileY != (TilesPerHeight/2)))){
						TileValue = 2;
					}
						
					if((TileY == 0) && (!DoorSouth || (TileX != (TilesPerWidth/2)))){
						TileValue = 2;
					}
					
					if((TileY == (TilesPerHeight - 1)) && (!DoorNorth || (TileX != (TilesPerWidth/2)))){
						TileValue = 2;
					}
					
					if((TileX == 6) && (TileY == 6)){
						if(DoorUp){
							TileValue = 3;
						}
						if(DoorDown){
							TileValue = 4;
						}
					}
					
					SetTileValue(&GameState->WorldArena, World->TileMap, AbsTileX, AbsTileY, AbsTileZ, TileValue);
				}
					
			}
			if(ZDoorMade){
				DoorDown = !DoorDown;
				DoorUp = !DoorUp;
			}else{
				DoorDown = false;
				DoorUp = false;
			}
			
			DoorWest = DoorEast;
			DoorSouth = DoorNorth;
			DoorEast = false;
			DoorNorth = false;
			if(RandomChoice == 0){
				if(AbsTileZ == 0){
					AbsTileZ = 1;
				}else{
					AbsTileZ = 0;
				}
			}else if(RandomChoice == 1){
				ScreenX += 1;
			}else{
				ScreenY += 1;
				
			}
		}
		
		Memory->IsInitilized = true;
	}
	
	world *World = GameState->World;
	tile_map *TileMap = World->TileMap;
	
	uint32 TileSideInPixels = 60; 
	real32 MetersToPixels = (real32)TileSideInPixels / (real32)TileMap->TileSideInMeters;
	
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
			
			tile_map_position NewPlayerPos = GameState->PlayerPos;
			NewPlayerPos.X += Input->dtForFrame * dPlayerX;
			NewPlayerPos.Y += Input->dtForFrame * dPlayerY;
			NewPlayerPos = RecanonicalizePosition(TileMap, NewPlayerPos);
			
			tile_map_position PlayerLeft = NewPlayerPos;
			PlayerLeft.X -= 0.5f*PlayerWidth;
			PlayerLeft = RecanonicalizePosition(TileMap, PlayerLeft);
			
			tile_map_position PlayerRight = NewPlayerPos;
			PlayerRight.X += 0.5f*PlayerWidth;
			PlayerRight = RecanonicalizePosition(TileMap, PlayerRight);
			
			tile_map_position PlayerTopHit = NewPlayerPos;
			PlayerTopHit.Y += 0.25f*PlayerHeight;
			PlayerTopHit = RecanonicalizePosition(TileMap, PlayerTopHit);
			
			if(IsTileMapPointEmpty(TileMap, NewPlayerPos) &&
			   IsTileMapPointEmpty(TileMap, PlayerLeft) &&
			   IsTileMapPointEmpty(TileMap, PlayerRight) &&
			   IsTileMapPointEmpty(TileMap, PlayerTopHit)
			){
				if(!AreOnSameTile(&GameState->PlayerPos, &NewPlayerPos)){
					uint32 TileVal = GetTileValue(TileMap, NewPlayerPos);
					
					if(TileVal == 3){
						++NewPlayerPos.AbsTileZ;
					}else if(TileVal == 4){
						--NewPlayerPos.AbsTileZ;
					}
				}
				GameState->PlayerPos = NewPlayerPos;
				
			}
		}
	}
	
	DrawRectangle(Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 1.0f);
	
	DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
	
	real32 ScreenCenterX = 0.5f * (real32)Buffer->Width;
	real32 ScreenCenterY = 0.5f * (real32)Buffer->Height;
	for(int32 RelRow = -6; RelRow < 6; ++RelRow){
		for(int32 RelCol = -10; RelCol < 10; ++RelCol){
			uint32 Col = RelCol + GameState->PlayerPos.AbsTileX;
			uint32 Row = RelRow + GameState->PlayerPos.AbsTileY;
			uint32 TileID = GetTileValue(TileMap, Col, Row, GameState->PlayerPos.AbsTileZ);
			if(TileID > 1){
				real32 Grey = 0.3f;
				if(TileID == 2){
					Grey = 0.9f;
				}
				
				if(TileID == 3){
					Grey = 0.2f;
				}
				if(TileID == 4){
					Grey = 0.2f;
				}
				
				if((Row == GameState->PlayerPos.AbsTileY) && (Col == GameState->PlayerPos.AbsTileX)){
					Grey = 0.0f;
				}
				
				real32 CenX = ScreenCenterX - MetersToPixels*GameState->PlayerPos.X + ((real32)RelCol) * TileSideInPixels;
				real32 CenY = ScreenCenterY + MetersToPixels*GameState->PlayerPos.Y - ((real32)RelRow) * TileSideInPixels;
				real32 MinX = CenX - 0.5f*TileSideInPixels;
				real32 MaxX = CenX + 0.5f*TileSideInPixels;
				real32 MinY = CenY - 0.5f*TileSideInPixels;
				real32 MaxY = CenY + 0.5f*TileSideInPixels;
				DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Grey, Grey, Grey);
			}
		}
	}
	
	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerLeft = ScreenCenterX - MetersToPixels * 0.5f * PlayerWidth;
	real32 PlayerTop =  ScreenCenterY - MetersToPixels * PlayerHeight;
	DrawBitmap(Buffer, &GameState->HeroHead, PlayerLeft, PlayerTop);
	
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples){
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

