
#include "core.h"
#include "core_world.cpp"
#include "core_random.h"


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
DrawRectangle(game_offscreen_buffer *Buffer, v2 Min, v2 Max, 
			  real32 R, real32 G, real32 B){
	int32 MinX = RoundReal32ToInt32(Min.X);
	int32 MinY = RoundReal32ToInt32(Min.Y);
	int32 MaxX = RoundReal32ToInt32(Max.X);
	int32 MaxY = RoundReal32ToInt32(Max.Y);
	
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
		
		uint32 RedMask = Header->RedMask;
        uint32 GreenMask = Header->GreenMask;
        uint32 BlueMask = Header->BlueMask;
        uint32 AlphaMask = ~(RedMask | GreenMask | BlueMask);        
        
        bit_scan_result RedScan = FindLeastSigSetBit(RedMask);
        bit_scan_result GreenScan = FindLeastSigSetBit(GreenMask);
        bit_scan_result BlueScan = FindLeastSigSetBit(BlueMask);
        bit_scan_result AlphaScan = FindLeastSigSetBit(AlphaMask);
        
        Assert(RedScan.Found);
        Assert(GreenScan.Found);
        Assert(BlueScan.Found);
        Assert(AlphaScan.Found);

        int32 RedShift = 16 - (int32)RedScan.Index;
        int32 GreenShift = 8 - (int32)GreenScan.Index;
        int32 BlueShift = 0 - (int32)BlueScan.Index;
        int32 AlphaShift = 24 - (int32)AlphaScan.Index;
        
        uint32 *SourceDest = Pixels;
        for(int32 Y = 0;
            Y < Header->Height;
            ++Y)
        {
            for(int32 X = 0;
                X < Header->Width;
                ++X)
            {
                uint32 C = *SourceDest;

                *SourceDest++ = (RotateLeft(C & RedMask, RedShift) |
                                 RotateLeft(C & GreenMask, GreenShift) |
                                 RotateLeft(C & BlueMask, BlueShift) |
                                 RotateLeft(C & AlphaMask, AlphaShift));
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

internal void
InitArena(memory_arena *Arena, memory_index Size, uint8 *Base){
	Arena->Size = Size;
	Arena->Base = Base;
	Arena->Used = 0;
}	

internal bool32 TestWall(real32 WallX, real32 RelX, real32 RelY, 
									 real32 PlayerDeltaX, real32 PlayerDeltaY, 
									 real32 MinY, real32 MaxY, real32 *tMin){
	bool32 Hit = false;
	real32 tEpsilon = 0.001f;
	if(PlayerDeltaX != 0.0f){
		real32 tResult = (WallX - RelX) / PlayerDeltaX;
		if((tResult >= 0.0f) && (tResult < *tMin)){
			real32 Y = RelY + tResult * PlayerDeltaY;
			if((Y >= MinY) && (Y <= MaxY)){
				*tMin = Maximum(0.0f, tResult-tEpsilon);
				Hit = true;
			}
		}
	}
	return(Hit);
}

internal void DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY, int32 AlignX = 0, int32 AlignY = 0){
	
	RealX -= (real32)AlignX;
	RealY -= (real32)AlignY;
	
	int32 MinX = RoundReal32ToInt32(RealX);
	int32 MaxX = MinX + Bitmap->Width;
	int32 MinY = RoundReal32ToInt32(RealY);
	int32 MaxY = MinY + Bitmap->Height;
	
	int32 SourceOffsetX = 0;
	int32 SourceOffsetY = 0;
	if(MinX < 0){
		SourceOffsetX = -MinX;
		MinX = 0;
	}
	if(MinY < 0){
		SourceOffsetY = -MinY;
		MinY = 0;
	}
	
	if(MaxX > Buffer->Width){
		MaxX = Buffer->Width;
	}
	if(MaxY > Buffer->Height){
		MaxY = Buffer->Height;
	}
	

	uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height-1);
	SourceRow += -Bitmap->Width*SourceOffsetY + SourceOffsetX;
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

inline v2 GetCameraSpacePos(game_state *GameState, external_entity *ExternalEntity){
	world_difference Diff = Subtract(GameState->World, &ExternalEntity->Pos, &GameState->CameraPos); 
	v2 Res = Diff.dXY;
	return(Res);
}

inline local_entity* ChangeEntityToLocal(game_state *GameState, uint32 ExternalIndex, external_entity *ExternalEntity, v2 CameraSpacePos){
	
	local_entity *LocalEntity = 0;
	Assert(ExternalEntity->LocalEntityIndex == 0);
	if(ExternalEntity->LocalEntityIndex == 0){
		if(GameState->LocalEntityCount < ArrayCount(GameState->LocalEntities)){
			uint32 LocalIndex = GameState->LocalEntityCount++;
			LocalEntity = GameState->LocalEntities + LocalIndex; 
			
			LocalEntity->Pos = CameraSpacePos;
			LocalEntity->dPos = V2(0,0);
			LocalEntity->ChunkZ = ExternalEntity->Pos.ChunkZ;
			LocalEntity->FacingDirection = 0;
			LocalEntity->ExternalEntityIndex = ExternalIndex;
			ExternalEntity->LocalEntityIndex = LocalIndex; 
		}else{
			InvalidCodePath
		}
	}
	return LocalEntity;
}

inline local_entity* ChangeEntityToLocal(game_state *GameState, uint32 ExternalIndex){
	local_entity *LocalEntity = 0;
	external_entity *ExternalEntity = GameState->ExternalEntities + ExternalIndex;
	if(ExternalEntity->LocalEntityIndex){
		LocalEntity = GameState->LocalEntities + ExternalEntity->LocalEntityIndex;
	} else {
		v2 CameraSpacePos = GetCameraSpacePos(GameState, ExternalEntity);
		LocalEntity = ChangeEntityToLocal(GameState, ExternalIndex, ExternalEntity, CameraSpacePos);
		
	}
	return LocalEntity;
}

inline void ChangeEntityToExternal(game_state *GameState, uint32 ExternalIndex){
	external_entity *ExternalEntity = &GameState->ExternalEntities[ExternalIndex];
	uint32 LocalIndex = ExternalEntity->LocalEntityIndex;
	if(LocalIndex){
		uint32 LastLocalIndex = GameState->LocalEntityCount - 1;
		if(LocalIndex != LastLocalIndex){
			local_entity *LastEntity = GameState->LocalEntities + LastLocalIndex;
			local_entity *DelEntity = GameState->LocalEntities + LocalIndex;
			*DelEntity = *LastEntity;
			GameState->ExternalEntities[LastEntity->ExternalEntityIndex].LocalEntityIndex = LocalIndex;
		}
		--GameState->LocalEntityCount;
		ExternalEntity->LocalEntityIndex = 0;
	}	
	
}

internal external_entity* GetExternalEntity(game_state *GameState, uint32 Index){
	external_entity *Res = 0;
	
	if((Index > 0) && (Index < GameState->ExternalEntityCount)){
		Res = GameState->ExternalEntities + Index;
	}
	return(Res);
}

internal entity GetLocalEntity(game_state *GameState, uint32 ExternalIndex){
	entity Res = {};
	
	if((ExternalIndex > 0) && (ExternalIndex < GameState->ExternalEntityCount)){
	
		Res.ExternalIndex = ExternalIndex;
		Res.External = GameState->ExternalEntities + ExternalIndex;
		Res.Local = ChangeEntityToLocal(GameState, ExternalIndex);
	}
	return(Res);
}

inline bool32 ValidateEntityPairs(game_state *GameState){
	bool32 Valid = true;
	for(uint32 LocalEntityIndex =1 ; LocalEntityIndex < GameState->LocalEntityCount; ++LocalEntityIndex){
		local_entity *LocalEntity = GameState->LocalEntities + LocalEntityIndex;
		Valid = Valid && (GameState->ExternalEntities[LocalEntity->ExternalEntityIndex].LocalEntityIndex == LocalEntityIndex);
	}
	return(Valid);
	
}

inline void OffsetAndCheckFrequencyByArea(game_state *GameState, v2 Offset, rectangle2 LocalBounds){
	for(uint32 LocalEntityIndex = 1; LocalEntityIndex < GameState->LocalEntityCount; ){
		local_entity *LocalEntity = GameState->LocalEntities + LocalEntityIndex;
		uint32 ExternalEntityIndex = LocalEntity->ExternalEntityIndex;
		LocalEntity->Pos += Offset;
		
		if(IsInRectangle(LocalBounds, LocalEntity->Pos)){
			++LocalEntityIndex;
		} else {
			Assert(GameState->ExternalEntities[ExternalEntityIndex].LocalEntityIndex == LocalEntityIndex);
			ChangeEntityToExternal(GameState, ExternalEntityIndex);
		}
	}
}

internal uint32 AddExternalEntity(game_state *GameState, entity_type Type, world_position *Pos){
	Assert(GameState->ExternalEntityCount < ArrayCount(GameState->ExternalEntities));
	uint32 Index = GameState->ExternalEntityCount++;
	external_entity *ExternalEntity = GameState->ExternalEntities + Index;
	*ExternalEntity = {};
	ExternalEntity->Type = Type;
	if(Pos){
		ExternalEntity->Pos = *Pos;
		ChangeEntityLocation(&GameState->WorldArena, GameState->World, Index, 0, Pos);
	}
	return(Index);
}

internal uint32 AddPlayer(game_state *GameState){
	world_position Pos = GameState->CameraPos;
	if(GameState->CameraFollowingEntityIndex){
		Pos.Offset_ += V2(2,0);
	}
	uint32 EntityIndex = AddExternalEntity(GameState, EntityType_Player, &Pos);
	external_entity *ExternalEntity = GetExternalEntity(GameState, EntityIndex);
	ExternalEntity->Height = 0.5f; //1.4f;
	ExternalEntity->Width = 1.0f; //*Entity->Height;
	ExternalEntity->Collidable = true;
	
	
	if(GameState->CameraFollowingEntityIndex == 0){
		GameState->CameraFollowingEntityIndex = EntityIndex;
	}
	return(EntityIndex);
}

internal uint32 AddWall(game_state *GameState, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ){
	world_position Pos = ChunkPositionFromTilePosition(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
	uint32 EntityIndex = AddExternalEntity(GameState, EntityType_Wall, &Pos);
	external_entity *ExternalEntity = GetExternalEntity(GameState, EntityIndex);
	
	ExternalEntity->Height = GameState->World->TileSideInMeters;
	ExternalEntity->Width = ExternalEntity->Height;
	ExternalEntity->Collidable = true;
	
	return(EntityIndex);
}


internal void MoveEntity(game_state *GameState, entity Entity, real32 dt, v2 ddPos){

	world *World = GameState->World;
	
	real32 ddPosLengthSq = LengthSq(ddPos);
	
	if(ddPosLengthSq > 1.0f){
		ddPos *= (1.0f/SquareRoot(ddPosLengthSq));
	}
	
	real32 PlayerSpeed = 50.0f;
	ddPos *= PlayerSpeed;
	
	//Friction
	ddPos += -7.0f*Entity.Local->dPos;
	
	v2 OldEntityPos = Entity.Local->Pos;
	v2 PlayerDelta = 0.5f * ddPos * Square(dt) + Entity.Local->dPos * dt;
	
	//meters/sec
	Entity.Local->dPos = ddPos * dt +	Entity.Local->dPos;
	
	v2 NewEntityPos = OldEntityPos + PlayerDelta;

	
	for(uint32 Iteration = 0 ; Iteration < 4 ; ++Iteration){
		v2 WallNormal = {};
		real32 tMin = 1.0f;
		uint32 HitLocalEntityIndex = 0;
		
		v2 TargetDestination = Entity.Local->Pos + PlayerDelta;
		for(uint32 TestEntityIndexLocal = 1; TestEntityIndexLocal < GameState->LocalEntityCount; ++TestEntityIndexLocal){
			if(TestEntityIndexLocal != Entity.External->LocalEntityIndex){
				
				entity TestEntity;
				TestEntity.Local = GameState->LocalEntities + TestEntityIndexLocal;
				TestEntity.ExternalIndex = TestEntity.Local->ExternalEntityIndex;
				TestEntity.External = GameState->ExternalEntities + TestEntity.ExternalIndex;
				if(TestEntity.External->Collidable){
					real32 DiameterW = TestEntity.External->Width + Entity.External->Width;
					real32 DiameterH = TestEntity.External->Height + Entity.External->Height;
					v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
					v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);
					
					v2 Rel = Entity.Local->Pos - TestEntity.Local->Pos;
					if(TestWall(MinCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, MinCorner.Y, MaxCorner.Y, &tMin)){
						WallNormal = V2(-1,0);
						HitLocalEntityIndex = TestEntityIndexLocal;
					}
					if(TestWall(MaxCorner.X, Rel.X, Rel.Y, PlayerDelta.X, PlayerDelta.Y, MinCorner.Y, MaxCorner.Y, &tMin)){
						WallNormal = V2(1, 0);
						HitLocalEntityIndex = TestEntityIndexLocal;
					}
					if(TestWall(MinCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, MinCorner.X, MaxCorner.X, &tMin)){
						WallNormal = V2(0, -1);
						HitLocalEntityIndex = TestEntityIndexLocal;
					}
					if(TestWall(MaxCorner.Y, Rel.Y, Rel.X, PlayerDelta.Y, PlayerDelta.X, MinCorner.X, MaxCorner.X, &tMin)){
						WallNormal = V2(0, 1);
						HitLocalEntityIndex = TestEntityIndexLocal;
					}
				}
			}
			
		}
		Entity.Local->Pos += tMin*PlayerDelta;
		if(HitLocalEntityIndex){
			Entity.Local->dPos = Entity.Local->dPos - 1*Inner(Entity.Local->dPos, WallNormal)*WallNormal;
			PlayerDelta = (TargetDestination - Entity.Local->Pos) - 1*Inner(PlayerDelta, WallNormal)*WallNormal;
			
			local_entity *HitLocalEntity = GameState->LocalEntities + HitLocalEntityIndex;
			external_entity *HitExternalEntity = GameState->ExternalEntities + HitLocalEntity->ExternalEntityIndex;
			//Entity.Local->AbsTileZ += HitExternalEntity->dAbsTileZ;
		}
		else{
			break;
		}
	}
	
	
	
	if((Entity.Local->dPos.X == 0.0f) && (Entity.Local->dPos.X == 0)){
		
	} else if(AbsoluteValue(Entity.Local->dPos.X) > AbsoluteValue(Entity.Local->dPos.Y)){
		// left/right
		if(Entity.Local->dPos.X > 0){
			Entity.Local->FacingDirection = 3;
		}else{
			Entity.Local->FacingDirection = 2;
		}
	}else if (AbsoluteValue(Entity.Local->dPos.X) < AbsoluteValue(Entity.Local->dPos.Y)){
		// up/down
		if(Entity.Local->dPos.Y > 0){
			Entity.Local->FacingDirection = 1;
		}else{
			Entity.Local->FacingDirection = 0;
		}
	}
	world_position NewPos = MapIntoChunkSpace(GameState->World, GameState->CameraPos, Entity.Local->Pos);
	ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.ExternalIndex, &Entity.External->Pos, &NewPos);
	Entity.External->Pos = NewPos;
}

internal void SetCamera(game_state *GameState, world_position NewCameraPos){
	world *World = GameState->World;
	world_difference dCameraPos = Subtract(World, &NewCameraPos, &GameState->CameraPos);
	GameState->CameraPos = NewCameraPos;
	
	v2 TileSpan = V2(17*3, 9*3);
	
	rectangle2 CameraBounds = RectCenterDim(V2(0,0), World->TileSideInMeters * TileSpan);
	v2 EntityOffsetForFrame = -dCameraPos.dXY;
	
	OffsetAndCheckFrequencyByArea(GameState, EntityOffsetForFrame, CameraBounds);
	
	world_position MinChunkPos = MapIntoChunkSpace(World, NewCameraPos, GetMinCorner(CameraBounds));
	world_position MaxChunkPos = MapIntoChunkSpace(World, NewCameraPos, GetMaxCorner(CameraBounds));
	
	for(int32 ChunkY = MinChunkPos.ChunkY; ChunkY <= MaxChunkPos.ChunkY; ++ChunkY){
		for(int32 ChunkX = MinChunkPos.ChunkX; ChunkX <= MaxChunkPos.ChunkX; ++ChunkX){
			
			world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, NewCameraPos.ChunkZ);
			if(Chunk){
				for(world_entity_block *Block = &Chunk->FirstBlock;Block; Block = Block->Next){
					for(uint32 BlockEntityIndex = 0; BlockEntityIndex < Block->EntityCount; ++BlockEntityIndex){
						external_entity *Ent = GameState->ExternalEntities + Block->ExternalEntityIndex[BlockEntityIndex];
						uint32 ExternalEntityIndex = Block->ExternalEntityIndex[BlockEntityIndex];
						if(Ent->LocalEntityIndex == 0){
							//load entities to local
							
							v2 CameraSpacePos = GetCameraSpacePos(GameState, Ent);
							if(IsInRectangle(CameraBounds, CameraSpacePos)){
								ChangeEntityToLocal(GameState, ExternalEntityIndex, Ent, CameraSpacePos);
							
							}
						}
					}
				}
			}
		}
	}
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender){
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitilized){
		InitArena(&GameState->WorldArena, Memory->PermanentStorageSize - sizeof(game_state), (uint8 *)Memory->PermanentStorage + sizeof(game_state)); 
		GameState->World = PushStruct(&GameState->WorldArena, world);
		world *World = GameState->World;
		
		AddExternalEntity(GameState, EntityType_Null, 0);
		GameState->LocalEntityCount = 1;
		
		GameState->Backdrop = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_background.bmp");
		
		GameState->Tree = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "tree00.bmp");
		
		hero_bitmaps *Bitmap;
		Bitmap = GameState->HeroBitmaps;
		
		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_front_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;
		
		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_back_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_back_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_back_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;
		
		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_left_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_left_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_left_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		++Bitmap;
		
		Bitmap->Head = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_right_head.bmp");
		Bitmap->Cape = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_right_cape.bmp");
		Bitmap->Torso = DEBUGLoadBMP(Thread, Memory->DEBUGPlatformReadEntireFile, "test_hero_right_torso.bmp");
		Bitmap->AlignX = 72;
		Bitmap->AlignY = 182;
		
		
		real32 TileSideInMeters = 1.4f;
		InitWorldMap(World, TileSideInMeters);
		
		uint32 TilesPerWidth = 17;
		uint32 TilesPerHeight = 9;
		
		uint32 ScreenBaseX = 0;		
		uint32 ScreenBaseY = 0;
		uint32 ScreenBaseZ = 0;
		uint32 ScreenY = ScreenBaseY;
		uint32 ScreenX = ScreenBaseX;
		uint32 AbsTileZ = ScreenBaseZ;
			
		bool32 DoorEast = false;
		bool32 DoorWest = false;
		bool32 DoorNorth = false;
		bool32 DoorSouth = false;
		bool32 DoorUp = false;
		bool32 DoorDown = false;
		
		uint32 RandomNumberIndex = 0;
		for(uint32 ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex){
			uint32 RandomChoice;
			//if(DoorUp || DoorDown) 
			{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2 + 1;
			}
			#if 0
			else{
				RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
			}
			#endif
			bool32 ZDoorMade = false;
			if (RandomChoice == 0){
				ZDoorMade = true;
				if(ScreenBaseZ == 0){
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
					
					if(TileValue == 2) {
						AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
					}
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
				if(AbsTileZ == ScreenBaseZ){
					AbsTileZ = ScreenBaseZ+1;
				}else{
					AbsTileZ = ScreenBaseZ;
				}
			}else if(RandomChoice == 1){
				ScreenX += 1;
			}else{
				ScreenY += 1;
				
			}
		}
		
		world_position NewCameraPos = {};
		NewCameraPos = ChunkPositionFromTilePosition(World, ScreenBaseX*TilesPerWidth + 17/2,ScreenBaseY*TilesPerHeight + 9/2, ScreenBaseZ);
		SetCamera(GameState, NewCameraPos);
		Memory->IsInitilized = true;
	}
	
	world *World = GameState->World;
	
	uint32 TileSideInPixels = 60; 
	real32 MetersToPixels = (real32)TileSideInPixels / (real32)World->TileSideInMeters;
	
	for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		uint32 ExternalIndex = GameState->PlayerIndexForController[ControllerIndex];
		if(ExternalIndex == 0){
			if(Controller->Start.EndedDown){
				uint32 EntityIndex = AddPlayer(GameState);
				GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
			}
		} else {
			entity ControllingEntity = GetLocalEntity(GameState, ExternalIndex);
			v2 ddEntity = {};
			if(Controller->IsAnalog){
				ddEntity = V2(Controller->StickAverageX, Controller->StickAverageY);
			} else {
				if (Controller->MoveUp.EndedDown){
					ddEntity.Y = 1.0f;
				}
				if (Controller->MoveDown.EndedDown){
					ddEntity.Y = -1.0f;
				}
				if (Controller->MoveLeft.EndedDown){
					ddEntity.X = -1.0f;
				}
				if (Controller->MoveRight.EndedDown){
					ddEntity.X = 1.0f;
				}
			}
			if(Controller->LeftShoulder.EndedDown){
				GameState->CameraFollowingEntityIndex = ControllingEntity.ExternalIndex;
			}
			MoveEntity(GameState, ControllingEntity, Input->dtForFrame, ddEntity);
		}
	}
	
	v2 EntityOffsetForFrame = {};
	entity CameraFollowingEntity = GetLocalEntity(GameState, GameState->CameraFollowingEntityIndex);
	if(CameraFollowingEntity.Local){
		world_position NewCameraPos = GameState->CameraPos;
		NewCameraPos.ChunkZ = CameraFollowingEntity.External->Pos.ChunkZ;
#if 0
		tile_map_difference Delta = Subtract(TileMap, &CameraFollowingEntity.External->Pos, &GameState->CameraPos);
		if(CameraFollowingEntity.Local->Pos.X > (9.0f * TileMap->TileSideInMeters)){
			NewCameraPos.AbsTileX += 17;
		}
		if(CameraFollowingEntity.Local->Pos.X < -(9.0f * TileMap->TileSideInMeters)){
			NewCameraPos.AbsTileX -= 17;
		}
		if(CameraFollowingEntity.Local->Pos.Y > (5.0f * TileMap->TileSideInMeters)){
			NewCameraPos.AbsTileY += 9;
		}
		if(CameraFollowingEntity.Local->Pos.Y < -(5.0f * TileMap->TileSideInMeters)){
			NewCameraPos.AbsTileY -= 9;
		}
		
#else
		NewCameraPos = CameraFollowingEntity.External->Pos;
#endif
		SetCamera(GameState, NewCameraPos);
	}else{
		
		//SetCamera(GameState, GameState->CameraPos);
	}
#if 1
	v2 TopLeft = V2(0.0f, 0.0f);
	v2 BotRight = V2((real32)Buffer->Width, (real32)Buffer->Height);
	DrawRectangle(Buffer, TopLeft, BotRight, 1.0f, 0.0f, 1.0f);
#else
	DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
#endif
	v2 ScreenCenter = {(real32)Buffer->Width, (real32)Buffer->Height};
	ScreenCenter *= 0.5f;
#if 0
	for(int32 RelRow = -6; RelRow < 6; ++RelRow){
		for(int32 RelCol = -10; RelCol < 10; ++RelCol){
			uint32 Col = RelCol + GameState->CameraPos.AbsTileX;
			uint32 Row = RelRow + GameState->CameraPos.AbsTileY;
			uint32 TileID = GetTileValue(TileMap, Col, Row, GameState->CameraPos.AbsTileZ);
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
				
				if((Row == GameState->CameraPos.AbsTileY) && (Col == GameState->CameraPos.AbsTileX)){
					Grey = 0.0f;
				}
				v2 TileSide = {(real32)TileSideInPixels, (real32)TileSideInPixels};
				TileSide *= 0.5f;
				v2 Cen = {ScreenCenter.X - MetersToPixels*GameState->CameraPos.Offset_.X + ((real32)RelCol) * TileSideInPixels,
						   ScreenCenter.Y + MetersToPixels*GameState->CameraPos.Offset_.Y - ((real32)RelRow) * TileSideInPixels};
				v2 Min = Cen - TileSide;
				v2 Max = Cen + TileSide;
				DrawRectangle(Buffer, Min, Max, Grey, Grey, Grey);
			}
		}
	}
#endif
	for(uint32 LocalEntityIndex = 1; LocalEntityIndex < GameState->LocalEntityCount; ++LocalEntityIndex){
		local_entity *LocalEntity = GameState->LocalEntities + LocalEntityIndex;
		external_entity *ExternalEntity = GameState->ExternalEntities + LocalEntity->ExternalEntityIndex;
		
		LocalEntity->Pos += EntityOffsetForFrame;
		//tile_map_difference Delta = Subtract(TileMap, &Entity->Pos, &GameState->CameraPos);
		real32 PlayerR = 1.0f;
		real32 PlayerG = 1.0f;
		real32 PlayerB = 0.0f;
		real32 PlayerGroundPointX = ScreenCenter.X + MetersToPixels * LocalEntity->Pos.X;
		real32 PlayerGroundPointY = ScreenCenter.Y - MetersToPixels * LocalEntity->Pos.Y;
		v2 PlayerLeftTop = {PlayerGroundPointX - MetersToPixels * 0.5f * ExternalEntity->Width,  
							PlayerGroundPointY - MetersToPixels * 0.5f * ExternalEntity->Height};
		v2 EntityWidthHeight = {ExternalEntity->Width * MetersToPixels, ExternalEntity->Height* MetersToPixels};
		
		if(ExternalEntity->Type == EntityType_Player){
			hero_bitmaps *HeroBitmap = &GameState->HeroBitmaps[LocalEntity->FacingDirection];
			DrawBitmap(Buffer, &HeroBitmap->Torso, PlayerGroundPointX, PlayerGroundPointY, HeroBitmap->AlignX, HeroBitmap->AlignY);
			DrawBitmap(Buffer, &HeroBitmap->Cape, PlayerGroundPointX, PlayerGroundPointY, HeroBitmap->AlignX, HeroBitmap->AlignY);
			DrawBitmap(Buffer, &HeroBitmap->Head, PlayerGroundPointX, PlayerGroundPointY, HeroBitmap->AlignX, HeroBitmap->AlignY);
			DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop+EntityWidthHeight , PlayerR, PlayerG, PlayerB);
		}else{
			
#if 0
			DrawRectangle(Buffer, PlayerLeftTop, PlayerLeftTop+EntityWidthHeight , PlayerR, (PlayerG+(10*LocalEntityIndex)), (PlayerB+(20*LocalEntityIndex)));
#else
			DrawBitmap(Buffer, &GameState->Tree, PlayerGroundPointX, PlayerGroundPointY, 0, 0);
#endif
		}
	}
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples){
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}

