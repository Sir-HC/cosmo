#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <malloc.h>

#include "core.h"
#include "win32_core.h"

#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub){
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub){
	return(ERROR_DEVICE_NOT_CONNECTED);
}
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_


#define DIRECT_SOUND_CREATE(name) HRESULT WINAPI name(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)
typedef DIRECT_SOUND_CREATE(direct_sound_create);


global_variable bool32 GlobalRunning;
global_variable bool32 GlobalPause;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalPerfCountFrequency;

//=======================CONTROL=======================

internal void
Win32LoadXInput(void){
	HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
	if(!XInputLibrary){
		XInputLibrary = LoadLibraryA("xinput1_3.dll");
	}
	if(!XInputLibrary){
		XInputLibrary = LoadLibraryA("xinput9_1_0.dll");
	}
	if(XInputLibrary){
		XInputGetState = (x_input_get_state *)GetProcAddress(XInputLibrary, "XInputGetState");
		XInputSetState = (x_input_set_state *)GetProcAddress(XInputLibrary, "XInputSetState");
	}
}

internal real32
Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold){
	real32 Result = 0;
	if((real32)Value < -DeadZoneThreshold){
		Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
	}
	else if ((real32)Value > DeadZoneThreshold){
		Result = (real32)((Value + DeadZoneThreshold) / (32768.0f - DeadZoneThreshold));
	}
	return(Result);
}

internal void
Win32ProcessXInputDigitalButtons(game_button_state *OldState, game_button_state *NewState, DWORD XInputButtonState, DWORD ButtonBit){
	NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
	NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown){
	Assert(NewState->EndedDown != IsDown);
	NewState->EndedDown = IsDown;
	++NewState->HalfTransitionCount;
}


//=======================Sound=======================

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize){
	HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

	if (DSoundLibrary){
		direct_sound_create *DirectSoundCreate = (direct_sound_create *) GetProcAddress(DSoundLibrary, "DirectSoundCreate");
		LPDIRECTSOUND DirectSound;
		if(DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0))){
			WAVEFORMATEX WaveFormat = {};
			WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
			WaveFormat.nChannels =2;
			WaveFormat.nSamplesPerSec = SamplesPerSecond;
			WaveFormat.wBitsPerSample = 16;
			WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample)/8;
			WaveFormat.nAvgBytesPerSec = WaveFormat.nSamplesPerSec*WaveFormat.nBlockAlign;
			WaveFormat.cbSize = 0;

			if(DirectSoundCreate, SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY))){
				DSBUFFERDESC BufferDescription = {};
				BufferDescription.dwSize = sizeof(BufferDescription);
				BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
				LPDIRECTSOUNDBUFFER PrimaryBuffer;
				if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0))){

					if(SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat))){
						OutputDebugStringA("Primary Buffer set\n");
					}
					else{

					}
				}
			}
			else{
			}
			DSBUFFERDESC BufferDescription = {};
			BufferDescription.dwSize = sizeof(BufferDescription);
			BufferDescription.dwFlags = DSBCAPS_GETCURRENTPOSITION2;
			BufferDescription.dwBufferBytes = BufferSize;
			BufferDescription.lpwfxFormat = &WaveFormat;

			if(SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &GlobalSecondaryBuffer, 0))){
				OutputDebugStringA("Secondary Buffer Set\n");
			}
		}

	}
	else{
		OutputDebugStringA("DSoundLibary Not Found\n");
	}
}

internal void
Win32FillSoundBuffer(win32_sound_output* SoundOutput, game_sound_output_buffer* SoundBuffer, DWORD ByteToLock, DWORD BytesToWrite) {
	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;

	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(
		ByteToLock,
		BytesToWrite,
		&Region1, &Region1Size,
		&Region2, &Region2Size, 0))) {

		int16* DestSample = (int16*)Region1;
		int16 *SourceSample = SoundBuffer->Samples;

		DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
		for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex) {
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}
		DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
		DestSample = (int16*)Region2;
		for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex) {
			*DestSample++ = *SourceSample++;
			*DestSample++ = *SourceSample++;
			++SoundOutput->RunningSampleIndex;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

internal void
Win32ClearSoundBuffer(win32_sound_output* SoundOutput){
	VOID* Region1;
	DWORD Region1Size;
	VOID* Region2;
	DWORD Region2Size;
	if (SUCCEEDED(GlobalSecondaryBuffer->Lock(
		0,
		SoundOutput->SecondaryBufferSize,
		&Region1, &Region1Size,
		&Region2, &Region2Size, 0))) {

		uint8 *DestSample = (uint8 *)Region1;
		for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex) {
			*DestSample++ = 0;
		}
		DestSample = (uint8 *)Region2;
		for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex) {
			*DestSample++ = 0;
		}

		GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
	}
}

//=======================WINDOW=======================

internal win32_window_dimension
Win32GetWindowDimension(HWND Window){
	win32_window_dimension Result;
	RECT ClientRect;
	GetClientRect(Window, &ClientRect);
	Result.Height = ClientRect.bottom - ClientRect.top;
	Result.Width = ClientRect.right - ClientRect.left;
	return(Result);
}

internal void
Win32ResizeDibSection(win32_offscreen_buffer *Buffer, int Width, int Height){

	if(Buffer->Memory){
		VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
	}

	Buffer->Width = Width;
	Buffer->Height = Height;
	Buffer->BytesPerPixel = 4;
	Buffer->Pitch = Width*Buffer->BytesPerPixel;

	Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
	Buffer->Info.bmiHeader.biWidth = Buffer->Width;
	// Top lefting
	Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
	Buffer->Info.bmiHeader.biPlanes = 1;
	Buffer->Info.bmiHeader.biBitCount = 32;
	Buffer->Info.bmiHeader.biCompression = BI_RGB;

	int BitmapMemorySize = Buffer->BytesPerPixel * Buffer->Width * Buffer->Height;
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferToWindow(win32_offscreen_buffer* Buffer, HDC DeviceContext, win32_window_dimension WindowDim){

	StretchDIBits(DeviceContext,
		0,0, WindowDim.Width, WindowDim.Height,
		0,0, Buffer->Width, Buffer->Height,
		Buffer->Memory,
		&Buffer->Info,
		DIB_RGB_COLORS, SRCCOPY
	);
}

//=======================FILES=======================

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory){
	if(Memory){
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile){
	debug_read_file_result Result = {};
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ,	0, OPEN_EXISTING, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE){
		LARGE_INTEGER FileSize;
		if(GetFileSizeEx(FileHandle, &FileSize)){
			uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
			Result.Content = VirtualAlloc(0, FileSize32, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			if (Result.Content){
				DWORD BytesRead;
				if(ReadFile(FileHandle, Result.Content, FileSize32, &BytesRead, 0) && (FileSize32 == BytesRead)){
					Result.ContentSize = FileSize32;
				}else{
					DEBUGPlatformFreeFileMemory(Result.Content);
					Result.Content = 0;
				}
			}
		}
	}

	CloseHandle(FileHandle);
	return(Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile){
	bool32 Result = false;
	HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE, 0,	0, CREATE_ALWAYS, 0, 0);
	if (FileHandle != INVALID_HANDLE_VALUE){
		DWORD BytesWritten;
		if(WriteFile(FileHandle, Memory, MemorySize, &BytesWritten, 0)){
			Result = (BytesWritten == MemorySize);
		}else{

		}
		CloseHandle(FileHandle);
	}

	return(Result);
}

//=======================MAINS=======================


inline FILETIME Win32GetLastWriteTime(char *SourceDLLName){
	FILETIME LastWriteTime = {};
	WIN32_FIND_DATA FindData;
		
	HANDLE FindHandle = FindFirstFileA(SourceDLLName, &FindData);
	if(FindHandle != INVALID_HANDLE_VALUE){
		LastWriteTime = FindData.ftLastWriteTime;
		FindClose(FindHandle);
	}
	return(LastWriteTime);
}

internal win32_game_code
Win32LoadGameCode(char *SourceDLLName, char *TempDLLName){
	win32_game_code Result = {};
	
	Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
	CopyFile(SourceDLLName, TempDLLName, FALSE);
	Result.GameCodeDLL = LoadLibraryA(TempDLLName);
	if(Result.GameCodeDLL){
		Result.UpdateAndRender = (game_update_and_render *)GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
		Result.GetSoundSamples = (game_get_sound_samples *)GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
		Result.IsValid = (Result.UpdateAndRender && Result.GetSoundSamples);
	}
	
	if(!Result.IsValid){
		Result.UpdateAndRender = GameUpdateAndRenderStub;
		Result.GetSoundSamples = GameGetSoundSamplesStub;
	}
	
	return(Result);
}

internal void Win32UnloadGameCode(win32_game_code *GameCode){
	if(GameCode->GameCodeDLL){
		FreeLibrary(GameCode->GameCodeDLL);
		GameCode->GameCodeDLL = 0;
	}
	
	GameCode->IsValid = false;
	GameCode->UpdateAndRender = GameUpdateAndRenderStub;
	GameCode->GetSoundSamples = GameGetSoundSamplesStub;
}

internal void
Win32ProcessPendingMessages(game_controller_input *KeyboardController){
	MSG Message;
	while (PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)){
		switch(Message.message){
			case WM_QUIT:
				GlobalRunning = false;
			case WM_SYSKEYDOWN:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_KEYUP:{
				uint32 VKCode = (uint32)Message.wParam;
				bool32 WasDown = ((Message.lParam & (1 << 30)) != 0);
				bool32 IsDown = ((Message.lParam & (1 << 31)) == 0);
				bool32 AltKeyWasDown = (Message.lParam & (1 << 29));
				if (WasDown != IsDown){
					if (AltKeyWasDown && VKCode == 'W') {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionUp, IsDown);
					}
					else if (AltKeyWasDown && VKCode == 'S') {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionDown, IsDown);
					}
					else if (AltKeyWasDown && VKCode == 'A') {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionLeft, IsDown);
					}
					else if (AltKeyWasDown && VKCode == 'D') {
						Win32ProcessKeyboardMessage(&KeyboardController->ActionRight, IsDown);
					}
					else if (VKCode == 'W') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
					}
					else if (VKCode == 'S') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
					}
					else if (VKCode == 'A') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
					}
					else if (VKCode == 'D') {
						Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
					}
					else if (VKCode == 'Q') {
						Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
					}
					else if (VKCode == 'E') {
						Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
					}
					else if (VKCode == VK_ESCAPE) {
						GlobalRunning = false;
					}
					else if (VKCode == VK_SPACE) {
						Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
					}
					else if (VKCode == 'P'){
						if(IsDown){
							GlobalPause = !GlobalPause;
						}
					}
				}

				if ((VKCode == VK_F4) && AltKeyWasDown){
					GlobalRunning = false;
				}
			}break;
			default:{
				TranslateMessage(&Message);
				DispatchMessageA(&Message);
			}
		}
	}
}

LRESULT CALLBACK Win32MainWindowCallback(HWND Window,
							        UINT Message,
							        WPARAM WParam,
							        LPARAM LParam){
	LRESULT Result = 0;
	switch(Message)
	{
		case WM_CREATE:
		{

		} break;

		case WM_SIZE:
		{

		} break;

		case WM_ACTIVATEAPP:
		{
			OutputDebugStringA("WM_ACTIVATEAPP\n");
		} break;

		case WM_PAINT:
		{
			PAINTSTRUCT Paint = {};

			HDC DeviceContext = BeginPaint(Window, &Paint);
			win32_window_dimension Dimension = Win32GetWindowDimension(Window);

			Win32DisplayBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension);
			//PatBlt(DeviceContext, X, Y, Width, Height, 0);
			EndPaint(Window, &Paint);
		} break;

		case WM_DESTROY:
		{
			GlobalRunning = false;
		} break;

		case WM_CLOSE:
		{
			GlobalRunning = false;
		} break;

		default:
		{
			//OutputDebugStringA("Default\n")
			Result = DefWindowProcA(Window, Message, WParam, LParam);
		} break;
	}

	return(Result);
}

inline LARGE_INTEGER
Win32GetWallClock(){
	LARGE_INTEGER Result;
	QueryPerformanceCounter(&Result);
	return(Result);
}

inline real32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End){
	real32 Result = ((real32)(End.QuadPart - Start.QuadPart) / (real32)GlobalPerfCountFrequency);
	return(Result);
}

internal void Win32DebugDrawVertical(win32_offscreen_buffer *Backbuffer, int X, int Top, int Bottom, uint32 Color) {
	if(Top <= 0){
		Top = 0;
	}
	if(Bottom >= Backbuffer->Height){
		Bottom = Backbuffer->Height;
	}
	
	if((X>= 0) && (X < Backbuffer->Width)){
		uint8 *Pixel = ((uint8 *)Backbuffer->Memory + X*Backbuffer->BytesPerPixel + Top*Backbuffer->Pitch);
		for(int Y = Top; Y < Bottom; ++Y){
			*(uint32 *)Pixel = Color;
			Pixel += Backbuffer->Pitch;
		}
	}
}

inline void 
Win32DrawSoundBufferMarker(win32_offscreen_buffer *Backbuffer, win32_sound_output *SoundOutput, real32 C, int PadX, int Top, int Bottom, DWORD Value, uint32 Color)
{
	real32 XReal32 = (C * (real32)Value);
	int X = PadX + (int)XReal32;
	
	Win32DebugDrawVertical(Backbuffer, X, Top, Bottom, Color);
}

internal void
Win32DebugSoundSyncDisplay(win32_offscreen_buffer *Backbuffer, int MarkerCount, win32_debug_time_marker *Markers, int CurrentMarkerIndex, win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame){
	int PadX = 16;
	int PadY = 16;
	int LineHeight = 64;
	
	real32 C = ((real32)Backbuffer->Width - 2*PadX) / (real32)SoundOutput->SecondaryBufferSize;
	for(int MarkerIndex = 0; MarkerIndex < MarkerCount ; ++MarkerIndex){
		win32_debug_time_marker *ThisMarker = &Markers[MarkerIndex];
		Assert(ThisMarker->OutputPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputWriteCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputLocation < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->OutputByteCount < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipPlayCursor < SoundOutput->SecondaryBufferSize);
		Assert(ThisMarker->FlipWriteCursor < SoundOutput->SecondaryBufferSize);
		
		DWORD PlayColor =  0xFFFFFFFF;
		DWORD WriteColor = 0xFF00FF00;
		DWORD ExpectedFlipColor = 0xFFFFFF00;
		int Top = PadY;
		int Bottom = PadY + LineHeight;
		if(MarkerIndex == CurrentMarkerIndex){
			int FirstTop = Top;
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			// This is what was going on just a little bit ago, just after calculating where to write and getting sound from game
			Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputPlayCursor, PlayColor);
			Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom+10, ThisMarker->OutputWriteCursor, WriteColor);
			
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			// This is what we put into the sound buffer beginning to end
			Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->OutputLocation, PlayColor);
			Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom+10, ThisMarker->OutputLocation + ThisMarker->OutputByteCount, WriteColor);
			
			Top += LineHeight + PadY;
			Bottom += LineHeight + PadY;
			Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, FirstTop, Bottom+20, ThisMarker->ExpectedFlipPlayCursor, ExpectedFlipColor);
		}
		// This is what the sound buffer was at after fliping the screen
		Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom, ThisMarker->FlipPlayCursor, PlayColor);
		Win32DrawSoundBufferMarker(Backbuffer, SoundOutput, C, PadX, Top, Bottom+10, ThisMarker->FlipWriteCursor, WriteColor);
	}
}

void CatStrings(size_t SourceACount, char *SourceA, size_t SourceBCount, char *SourceB, size_t DestCount, char *Dest){
	for(int Index = 0; Index < SourceACount; ++Index){
		*Dest++ = *SourceA++;
	}
	for(int Index = 0; Index < SourceBCount; ++Index){
		*Dest++ = *SourceB++;
	}
	*Dest++ = 0;
}

int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CommandLine,
					 int ShowCode)
{
	char EXEFilePath[MAX_PATH];
	DWORD SizeOfFilename = GetModuleFileName(0,EXEFilePath,sizeof(EXEFilePath));
	char *FoundFilename = EXEFilePath;
	for(char *Scan=EXEFilePath; *Scan; ++Scan){
		if(*Scan == '\\'){
			FoundFilename = Scan + 1;
		}
	}
	
	char GameCodeDLLFilename[] = "core.dll";
	char GameCodeDLLPath[MAX_PATH];
	CatStrings(FoundFilename - EXEFilePath, EXEFilePath, 
				sizeof(GameCodeDLLFilename) - 1, GameCodeDLLFilename, 
				sizeof(GameCodeDLLPath), GameCodeDLLPath );
	
	char TempGameCodeDLLFilename[] = "core_temp.dll";
	char TempGameCodeDLLPath[MAX_PATH];
	CatStrings(FoundFilename - EXEFilePath, EXEFilePath, 
				sizeof(TempGameCodeDLLFilename) - 1, TempGameCodeDLLFilename, 
				sizeof(TempGameCodeDLLPath), TempGameCodeDLLPath );
	
	LARGE_INTEGER PerfCountFrequencyResult;
	QueryPerformanceFrequency(&PerfCountFrequencyResult);
	GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;

	UINT DesiredSchedulerMS = 1;
	bool32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);

	Win32LoadXInput();
	WNDCLASSA WindowClass = {};

	Win32ResizeDibSection(&GlobalBackBuffer, 1280, 720);

	WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
//	WindowClass.hIcon;
	WindowClass.lpszClassName = "CosmoWindowClass";

#define MonitorRefreshHz 60
#define GameUpdateHz (MonitorRefreshHz / 2)
#define FramesOfAudioLatency 3

	real32 TargetSecondsPerFrame = 1.0f / (real32)GameUpdateHz;

	if(RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowExA(
		  0,
		  WindowClass.lpszClassName,
		  "Engine",
		  WS_OVERLAPPEDWINDOW|WS_VISIBLE,
		  CW_USEDEFAULT,
		  CW_USEDEFAULT,
		  CW_USEDEFAULT,
		  CW_USEDEFAULT,
		  0,
		  0,
		  Instance,
		  0);
		if(Window){

			HDC DeviceContext = GetDC(Window);

			int XOffset = 0;
			int YOffset = 0;

			bool32 SoundIsPlaying = false;
			bool32 SoundIsValid = true;

			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.BytesPerSample = sizeof(int16) * 2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = 3*(SoundOutput.SamplesPerSecond/GameUpdateHz);
			SoundOutput.SafetyBytes = (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample / GameUpdateHz) / 3;
			Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
			Win32ClearSoundBuffer(&SoundOutput);
			GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

			int16 *Samples = (int16 *)VirtualAlloc(0, SoundOutput.SecondaryBufferSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
 
			game_memory GameMemory = {};
			GameMemory.IsInitilized = false;
#if CORE_INTERNAL
			LPVOID BaseAddress = (LPVOID)Terabytes((uint64)2);
#else
			LPVOID BaseAddress = 0;
#endif
			GameMemory.PermanentStorageSize = Megabytes(64);
			GameMemory.TransientStorageSize = Gigabytes(uint64(1));
			
			GameMemory.DEBUGPlatformReadEntireFile = 0;
			GameMemory.DEBUGPlatformFreeFileMemory = 0;
			GameMemory.DEBUGPlatformWriteEntireFile = 0;
			
			uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

			GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			GameMemory.TransientStorage = (uint8 *)GameMemory.PermanentStorage + GameMemory.TransientStorageSize;

			GlobalRunning = true;

			if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage){

				game_input Input[2] = {};
				game_input *NewInput = &Input[0];
				game_input *OldInput = &Input[1];
				LARGE_INTEGER LastCounter = Win32GetWallClock();
				LARGE_INTEGER FlipWallClock = LastCounter;

				win32_game_code Game = Win32LoadGameCode(GameCodeDLLPath, TempGameCodeDLLPath);
				int64 LastCycleCount = __rdtsc();

				int DebugTimeMarkerIndex = 0;
				win32_debug_time_marker DebugTimeMarkers[GameUpdateHz / 2]  = {0};
				while(GlobalRunning){
					
					FILETIME NewDLLWriteTime = Win32GetLastWriteTime(GameCodeDLLPath);
					if(CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0){
						Win32UnloadGameCode(&Game);
						Game = Win32LoadGameCode(GameCodeDLLPath, TempGameCodeDLLPath);
						
					}
					game_controller_input *OldKeyboardController = GetController(OldInput, 0);
					game_controller_input *NewKeyboardController = GetController(NewInput, 0);
					*NewKeyboardController = {};
					NewKeyboardController->IsConnected = true;
					for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons);++ButtonIndex){
						NewKeyboardController->Buttons[ButtonIndex].EndedDown =
							OldKeyboardController->Buttons[ButtonIndex].EndedDown;
					}

					Win32ProcessPendingMessages(NewKeyboardController);
					
					if(!GlobalPause){
						DWORD MaxControllerCount = XUSER_MAX_COUNT;
						if(MaxControllerCount > (ArrayCount(Input->Controllers) - 1)){
								MaxControllerCount = (ArrayCount(Input->Controllers) - 1);
						}

						for(DWORD ControllerIndex=0 ; ControllerIndex < MaxControllerCount; ++ControllerIndex)	{
							int32 OurControllerIndex = ControllerIndex + 1;
							game_controller_input *OldController = GetController(OldInput, OurControllerIndex);
							game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
							XINPUT_STATE ControllerState;
							if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS){
								// controller is plugged in
								// check ControllerState.dwPackerNumber changes a lot
								XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;
								NewController->IsConnected = true;
								NewController->IsAnalog = false;
								NewController->StickAverageX = Win32ProcessXInputStickValue(Pad->sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
								NewController->StickAverageY = Win32ProcessXInputStickValue(Pad->sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
								if((NewController->StickAverageX != 0.0f) || (NewController->StickAverageY != 0.0f)){
									NewController->IsAnalog = true;
								}

								real32 Threshold = 0.5f;
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP){
									NewController->StickAverageY = 1;
								}
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN){
									NewController->StickAverageY = -1;
								}
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT){
									NewController->StickAverageX = -1;
								}
								if(Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT){
									NewController->StickAverageX = 1;
								}

								Win32ProcessXInputDigitalButtons(&OldController->MoveUp,    &NewController->MoveUp,    (NewController->StickAverageY >  Threshold) ? 1: 0, 1);
								Win32ProcessXInputDigitalButtons(&OldController->MoveDown,  &NewController->MoveDown,  (NewController->StickAverageY < -Threshold) ? 1: 0, 1);
								Win32ProcessXInputDigitalButtons(&OldController->MoveLeft,  &NewController->MoveLeft,  (NewController->StickAverageX >  Threshold) ? 1: 0, 1);
								Win32ProcessXInputDigitalButtons(&OldController->MoveRight, &NewController->MoveRight, (NewController->StickAverageX < -Threshold) ? 1: 0, 1);

								Win32ProcessXInputDigitalButtons(&OldController->ActionDown,  &NewController->ActionDown,  Pad->wButtons, XINPUT_GAMEPAD_A);
								Win32ProcessXInputDigitalButtons(&OldController->ActionRight, &NewController->ActionRight, Pad->wButtons, XINPUT_GAMEPAD_B);
								Win32ProcessXInputDigitalButtons(&OldController->ActionLeft,  &NewController->ActionLeft,  Pad->wButtons, XINPUT_GAMEPAD_X);;
								Win32ProcessXInputDigitalButtons(&OldController->ActionUp,    &NewController->ActionUp,    Pad->wButtons, XINPUT_GAMEPAD_Y);

								Win32ProcessXInputDigitalButtons(&OldController->LeftShoulder,  &NewController->LeftShoulder,  Pad->wButtons, XINPUT_GAMEPAD_LEFT_SHOULDER);
								Win32ProcessXInputDigitalButtons(&OldController->RightShoulder, &NewController->RightShoulder, Pad->wButtons, XINPUT_GAMEPAD_RIGHT_SHOULDER);

								Win32ProcessXInputDigitalButtons(&OldController->Start,  &NewController->Start,  Pad->wButtons, XINPUT_GAMEPAD_START);
								Win32ProcessXInputDigitalButtons(&OldController->RightShoulder, &NewController->RightShoulder, Pad->wButtons, XINPUT_GAMEPAD_BACK);

								bool32 LeftThumb = (Pad->wButtons & XINPUT_GAMEPAD_LEFT_THUMB);
								bool32 RightThumb = (Pad->wButtons & XINPUT_GAMEPAD_RIGHT_THUMB);

							}
							else{
								NewController->IsConnected = false;
							}
						}
						
						game_offscreen_buffer Buffer = {};
						Buffer.Memory = GlobalBackBuffer.Memory;
						Buffer.Width = GlobalBackBuffer.Width;
						Buffer.Height = GlobalBackBuffer.Height;
						Buffer.Pitch = GlobalBackBuffer.Pitch;
						

						Game.UpdateAndRender(&GameMemory, NewInput, &Buffer);
						
						LARGE_INTEGER AudioWallClock = Win32GetWallClock();
						real32 FromBeginToAudioSeconds = (1000.0f*Win32GetSecondsElapsed(FlipWallClock, AudioWallClock));
						
						DWORD PlayCursor;
						DWORD WriteCursor;
						if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK){
							
							/*
						
							When we wake to write audio we will look for play cursor position.
							Forcast ahead where we think the play cursor will be on the next frame boundry.
							Look to see where the write cursor is in relation to that.
								If before frame boundry, write to the that frame boundry, then one frame more.
								
								If after frame boundry, then we will write one frame + samples to guard against the latency
							*/
				
							if(!SoundIsValid)
							{
								SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
								SoundIsValid = true;
							}
							DWORD ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
							
							
							DWORD ExpectedSoundBytesPerFrame = (SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample) / GameUpdateHz;
							real32 SecondsLeftUntilFlip = (TargetSecondsPerFrame - FromBeginToAudioSeconds);
							DWORD ExpectedBytesUntilFlip = (DWORD)((SecondsLeftUntilFlip/TargetSecondsPerFrame) * (real32)ExpectedSoundBytesPerFrame);
							
							DWORD ExpectedFrameBoundryByte = PlayCursor + ExpectedSoundBytesPerFrame;
							
							DWORD SafeWriteCursor = WriteCursor;
							if(SafeWriteCursor < PlayCursor){
								SafeWriteCursor += SoundOutput.SecondaryBufferSize;
							}
							Assert(SafeWriteCursor >= PlayCursor);
								
							SafeWriteCursor += SoundOutput.SafetyBytes;
							bool32 AudioCardLowLatency = (SafeWriteCursor < ExpectedFrameBoundryByte);
							
							DWORD TargetCursor = 0;
							if(AudioCardLowLatency){
								TargetCursor = ExpectedFrameBoundryByte + ExpectedSoundBytesPerFrame;
							}else{
								TargetCursor = WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes;
							}
							TargetCursor = TargetCursor % SoundOutput.SecondaryBufferSize;
							
							DWORD BytesToWrite = 0;
							if(ByteToLock > TargetCursor){
								BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
								BytesToWrite += TargetCursor;
							}
							else{
								BytesToWrite = TargetCursor - ByteToLock;
							}
							
							game_sound_output_buffer SoundBuffer = {};
							SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
							SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
							SoundBuffer.Samples = Samples;
							Game.GetSoundSamples(&GameMemory, &SoundBuffer);
							
#if CORE_INTERNAL
							win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
							Marker->OutputPlayCursor = PlayCursor;
							Marker->OutputWriteCursor = WriteCursor;
							Marker->OutputLocation = ByteToLock;
							Marker->OutputByteCount = BytesToWrite;
							Marker->ExpectedFlipPlayCursor = ExpectedBytesUntilFlip;
							
							DWORD UnwrappedWriteCursor = WriteCursor;
							if (UnwrappedWriteCursor < PlayCursor){
								UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
							}
							DWORD AudioLatencyBytes = WriteCursor - PlayCursor;
							real32 AudioLatencySeconds = (((real32)AudioLatencyBytes / (real32)SoundOutput.BytesPerSample) /
															(real32)SoundOutput.SamplesPerSecond);
							char sbuf[256];
							sprintf(sbuf, "BTL:%u, TC:%u, BTW:%u - PC:%u WC:%u Delta:%u (%fs)\n", ByteToLock, TargetCursor, BytesToWrite, PlayCursor, WriteCursor, AudioLatencyBytes, AudioLatencySeconds);
							OutputDebugStringA(sbuf);
#endif
							Win32FillSoundBuffer(&SoundOutput, &SoundBuffer, ByteToLock, BytesToWrite);
						}
						else{
							SoundIsValid = false;
						}
						
						
						LARGE_INTEGER WorkCounter = Win32GetWallClock();

						real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
						real32 SecondsElapsedForFrame = WorkSecondsElapsed;

						if(SecondsElapsedForFrame < TargetSecondsPerFrame){
							if(SleepIsGranular){
								DWORD SleepMS = (DWORD)((TargetSecondsPerFrame - SecondsElapsedForFrame) * 1000.f);
								if(SleepMS > 0){
									Sleep(SleepMS);
								}
							}
							real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
							//Assert(TestSecondsElapsedForFrame < TargetSecondsPerFrame);
							if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
							{
								// TODO Log missed sleep
							}
							while(SecondsElapsedForFrame < TargetSecondsPerFrame){
								SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
							}
						}
						else{
						}
						LARGE_INTEGER EndCounter = Win32GetWallClock();
						real32 MSPerFrame = (1000.0f*Win32GetSecondsElapsed(LastCounter, EndCounter));
						LastCounter = EndCounter;

						win32_window_dimension Dimension = Win32GetWindowDimension(Window);
#if CORE_INTERNAL
						Win32DebugSoundSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarkers), DebugTimeMarkers, DebugTimeMarkerIndex - 1, &SoundOutput, TargetSecondsPerFrame);
#endif
						Win32DisplayBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension);
						
						
#if CORE_INTERNAL
						
						{
							//DWORD PlayCursor;
							//DWORD WriteCursor;
							if(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK){
								Assert(DebugTimeMarkerIndex < ArrayCount(DebugTimeMarkers));
								win32_debug_time_marker *Marker = &DebugTimeMarkers[DebugTimeMarkerIndex];
								Marker->FlipPlayCursor = PlayCursor;
								Marker->FlipWriteCursor = WriteCursor;
							}
							
						}
#endif

						game_input *Temp = NewInput;
						NewInput = OldInput;
						OldInput = Temp;


						real64 FPS = 0.0f;

						int64 EndCycleCount = __rdtsc();
						int64 CyclesElapsed = EndCycleCount - LastCycleCount;
						LastCycleCount = EndCycleCount;

						real64 MegaCycles = (real64)(CyclesElapsed / (1000 * 1000));

						char buf[256];
						sprintf(buf, "MS: %.02fms, FPS: %.02lf, MCycles: %.02lf\n", MSPerFrame, FPS, MegaCycles);
						OutputDebugStringA(buf);
						
#if CORE_INTERNAL
						++DebugTimeMarkerIndex;
						if(DebugTimeMarkerIndex == ArrayCount(DebugTimeMarkers)){
							DebugTimeMarkerIndex = 0;
						}
#endif
					}
				}
			}
			else{
			}
		}
		else{
		}
	}
	else{
	}


	return(0);
 }
