#include <stdint.h>


#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265

#define XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE 8689
#define XINPUT_GAMEPAD_TRIGGER_THRESHOLD 30

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef float real32;
typedef double real64;


#include <math.h>

#include "core.cpp"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>
#include <malloc.h>

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



internal debug_read_file_result
DEBUGPlatformReadEntireFile(char *Filename){
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

internal void
DEBUGPlatformFreeFileMemory(void *Memory){
	if(Memory){
		VirtualFree(Memory, 0, MEM_RELEASE);
	}
}



internal bool32
DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory){
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

internal void Win32DebugDrawVertical(win32_offscreen_buffer *GlobalBackbuffer, int X, int Top, int Bottom, uint32 Color) {
	uint8 *Pixel = ((uint8 *)GlobalBackbuffer->Memory + X*GlobalBackbuffer->BytesPerPixel + Top*GlobalBackbuffer->Pitch);
	for(int Y = Top; Y < Bottom; ++Y){
		*(uint32 *)Pixel = Color;
		Pixel += GlobalBackbuffer->Pitch;
	}
}

internal void
Win32DebugSyncDisplay(win32_offscreen_buffer *GlobalBackbuffer, int LastPlayCursorCount, DWORD *LastPlayCursor, win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame){
	int PadX = 16;
	int PadY = 16;
	int Top = PadY;
	int Bottom = GlobalBackbuffer->Height - PadY;
	real32 C = ((real32)GlobalBackbuffer->Width - 2*PadX) / (real32)SoundOutput->SecondaryBufferSize;
	for(int PlayCursorIndex = 0; PlayCursorIndex < LastPlayCursorCount ; ++PlayCursorIndex){
		int X = PadX + (int)(C * (real32)LastPlayCursor[PlayCursorIndex]);
		Win32DebugDrawVertical(GlobalBackbuffer, X, Top, Bottom, 0xFFFFFFFF);
	}
}

int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CommandLine,
					 int ShowCode)
{
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

			win32_sound_output SoundOutput = {};
			SoundOutput.SamplesPerSecond = 48000;
			SoundOutput.RunningSampleIndex = 0;
			SoundOutput.BytesPerSample = sizeof(int16) * 2;
			SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSecond * SoundOutput.BytesPerSample;
			SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSecond/15;
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
			uint64 TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;

			GameMemory.PermanentStorage = VirtualAlloc(BaseAddress, TotalSize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
			GameMemory.TransientStorage = (uint8 *)GameMemory.PermanentStorage + GameMemory.TransientStorageSize;

			GlobalRunning = true;

			if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage){

				game_input Input[2] = {};
				game_input *NewInput = &Input[0];
				game_input *OldInput = &Input[1];
				LARGE_INTEGER LastCounter = Win32GetWallClock();

				int64 LastCycleCount = __rdtsc();

				int DebugLastPlayCursorIndex = 0;
				DWORD DebugLastPlayCursor[GameUpdateHz / 2]  = {0};
				while(GlobalRunning){

					game_controller_input *OldKeyboardController = GetController(OldInput, 0);
					game_controller_input *NewKeyboardController = GetController(NewInput, 0);
					*NewKeyboardController = {};
					NewKeyboardController->IsConnected = true;
					for(int ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons);++ButtonIndex){
						NewKeyboardController->Buttons[ButtonIndex].EndedDown =
							OldKeyboardController->Buttons[ButtonIndex].EndedDown;
					}

					Win32ProcessPendingMessages(NewKeyboardController);

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

					DWORD PlayCursor;
					DWORD WriteCursor;
					bool32 SoundIsValid = false;
					DWORD ByteToLock = 0;
					DWORD TargetCursor;
					DWORD BytesToWrite = 0;
					if(SUCCEEDED(GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor))){
						ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
						TargetCursor = (PlayCursor + (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample)) % SoundOutput.SecondaryBufferSize;
						if(ByteToLock > TargetCursor){
							BytesToWrite = SoundOutput.SecondaryBufferSize - ByteToLock;
							BytesToWrite += TargetCursor;
						}
						else{
							BytesToWrite = TargetCursor - ByteToLock;
						}
						SoundIsValid = true;
					}

					game_sound_output_buffer SoundBuffer = {};
					SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
					SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
					SoundBuffer.Samples = Samples;

					game_offscreen_buffer Buffer = {};
					Buffer.Memory = GlobalBackBuffer.Memory;
					Buffer.Width = GlobalBackBuffer.Width;
					Buffer.Height = GlobalBackBuffer.Height;
					Buffer.Pitch = GlobalBackBuffer.Pitch;


					GameUpdateAndRender(&GameMemory, NewInput, &Buffer, &SoundBuffer);


					if(SoundIsValid){
						Win32FillSoundBuffer(&SoundOutput, &SoundBuffer, ByteToLock, BytesToWrite);
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
						//real32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
						//Assert(TestSecondsElapsedForFrame < TargetSecondsPerFrame);
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
					Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugLastPlayCursor), DebugLastPlayCursor, &SoundOutput, TargetSecondsPerFrame);
#endif
					Win32DisplayBufferToWindow(&GlobalBackBuffer, DeviceContext, Dimension);

#if CORE_INTERNAL
					{
					  DWORD PlayCursor_;
						DWORD WriteCursor_;
						GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor_, &WriteCursor_);
						DebugLastPlayCursor[DebugLastPlayCursorIndex++] = PlayCursor_;
						if(DebugLastPlayCursorIndex > ArrayCount(DebugLastPlayCursor)){
							DebugLastPlayCursorIndex = 0;
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
				}
			}
		}
	}
	else{
	}


	return(0);
 }
