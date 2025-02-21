#if !defined(WIN32_CORE_H)

struct win32_offscreen_buffer
{
	BITMAPINFO Info;
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct win32_window_dimension
{
	int Width;
	int Height;
};

struct win32_sound_output
{
	int SamplesPerSecond;
	uint32 RunningSampleIndex;
	int BytesPerSample;
	DWORD SecondaryBufferSize;
	DWORD SafetyBytes;
	int LatencySampleCount;
	real32 tSine;
};

struct win32_debug_time_marker
{
	DWORD OutputPlayCursor;
	DWORD OutputWriteCursor;
	DWORD OutputLocation;
	DWORD OutputByteCount;
	DWORD ExpectedFlipPlayCursor;
	
	DWORD FlipPlayCursor;
	DWORD FlipWriteCursor; 
};

struct win32_game_code{
	HMODULE GameCodeDLL;
	FILETIME DLLLastWriteTime;
	game_update_and_render *UpdateAndRender;
	game_get_sound_samples *GetSoundSamples;
	
	bool32 IsValid;
};

struct win32_state{
	void *GameMemoryBlock;
	uint64 TotalSize;
	HANDLE RecordHandle;
	int InputRecordingIndex;
	HANDLE PlayRecordHandle;
	int InputPlayRecordIndex;
	
};

#define WIN32_CORE_H
#endif
