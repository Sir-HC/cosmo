#if !defined(CORE)


//PERFORMANCE_SLOW:
// 		1 - Slow code
//      0 - No slow code
//CORE_INTERNAL:
//      1 - Developer build
//      0 - Shipping build

#if PERFORMANCE_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif


#define ArrayCount(Array) (sizeof(Array)/ sizeof((Array)[0]))

#define Kilobytes(Value) (Value*1024)
#define Megabytes(Value) (Kilobytes(Value)*1024)
#define Gigabytes(Value) (Megabytes(Value)*1024)
#define Terabytes(Value) (Gigabytes(Value)*1024)

#if CORE_INTERNAL
struct debug_read_file_result{
	uint32 ContentSize;
	void *Content;
};
internal debug_read_file_result DEBUGPlatformReadEntireFile(char *Filename);
internal void DEBUGPlatformFreeFileMemory(void *Memory);

internal bool32 DEBUGPlatformWriteEntireFile(char *Filename, uint32 MemorySize, void *Memory);
#endif

inline uint32
SafeTruncateUInt64(uint64 Value){
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return(Result);
}


struct game_offscreen_buffer
{
	void *Memory;
	int Width;
	int Height;
	int Pitch;
	int BytesPerPixel;
};

struct game_sound_output_buffer{
	int16 *Samples;
	int SampleCount;
	int SamplesPerSecond;
};

struct game_button_state{
	int HalfTransitionCount;
	bool32 EndedDown;
};


struct game_controller_input{
	bool32 IsConnected;
	bool32 IsAnalog;
	real32 StickAverageX;
	real32 StickAverageY;
	
	union{
		game_button_state Buttons[12];
		struct{
			game_button_state MoveUp;
			game_button_state MoveDown;
			game_button_state MoveLeft;
			game_button_state MoveRight;
			
			game_button_state ActionUp;
			game_button_state ActionDown;
			game_button_state ActionLeft;
			game_button_state ActionRight;
			
			game_button_state LeftShoulder;
			game_button_state RightShoulder;
			
			game_button_state Back;
			game_button_state Start;
			
			
			
			game_button_state Terminator;
		};
	};
};

struct game_input{
	game_controller_input Controllers[4];
};

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex){
	Assert(ControllerIndex < ArrayCount(Input->Controllers));
	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return(Result);
}


struct game_memory{
	bool32 IsInitilized;
	uint64 PermanentStorageSize;
	void *PermanentStorage;	//REQUIRED to be zeroed out at start
	uint64 TransientStorageSize;
	void *TransientStorage; //REQUIRED to be zeroed out at start
};

struct game_state{
	int ToneHz;
	int GreenOffset;
	int BlueOffset;
};


internal void GameUpdateAndRender(game_input *Input, game_offscreen_buffer *Buffer,  game_sound_output_buffer *SoundBuffer);

#define CORE
#endif