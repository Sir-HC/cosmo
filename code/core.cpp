
#include "core.h"

internal void 
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	int16 ToneVolume = 600;
	int16* SampleOut = SoundBuffer->Samples;
	
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

	for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex) {
		real32 SineValue = sinf(GameState->tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;
		
		GameState->tSine += (real32)(2.0f * Pi32 * 1.0f) / (real32)WavePeriod;
		if(GameState->tSine > 2.0f*Pi32){
			GameState->tSine -= (real32)(2.0f*Pi32);
		}
	}
}
	
	
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


extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender){
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) == (ArrayCount(Input->Controllers[0].Buttons)));
	
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if(!Memory->IsInitilized){
		/*
		char *Filename = __FILE__;
		debug_read_file_result File = Memory->DEBUGPlatformReadEntireFile(Filename);
		if(File.Content){
			Memory->DEBUGPlatformWriteEntireFile("test.out", File.ContentSize, File.Content);
			Memory->DEBUGPlatformFreeFileMemory(File.Content);
		}
		*/		
		
		GameState->ToneHz = 256; 
		GameState->tSine = 0.0f;
		Memory->IsInitilized = true;
	}
	
	for(int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if(Controller->IsAnalog){
			GameState->ToneHz = 256 + (int)(128.0f * Controller->StickAverageY);
			
			GameState->BlueOffset += (int)(4.0f*Controller->StickAverageX);
		}
		else{
			
		}
		if(Controller->MoveDown.EndedDown){
			GameState->GreenOffset -= 1;
		}
		if(Controller->MoveUp.EndedDown){
			GameState->GreenOffset += 1;
		}
		if(Controller->MoveRight.EndedDown){
			GameState->BlueOffset -= 1;
		}
		if(Controller->MoveLeft.EndedDown){
			GameState->BlueOffset += 1;
		}
		
	}
	
	RenderWeirdGradient(Buffer, GameState->BlueOffset, GameState->GreenOffset);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples){
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, GameState->ToneHz);
}

