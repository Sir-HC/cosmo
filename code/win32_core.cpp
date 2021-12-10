#include <windows.h>
#include <stdint.h>

#define internal static
#define local_persist static
#define global_variable static 

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;

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

global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;


internal void
RenderWeirdGradient(win32_offscreen_buffer Buffer, int XOffset, int YOffset)
{	
	uint8 *Row = (uint8 *)Buffer.Memory;
	
	for(int Y = 0;
		Y < Buffer.Height;
		++Y){
		uint32 *Pixel = (uint32 *)Row;
		for(int X = 0;
			X < Buffer.Width;
			++X){
			uint8 Blue = (X+XOffset);
			uint8 Green = (Y+YOffset);
			uint8 Red = (X+Y);
			*Pixel++ = (((Red << 16) | Green << 8) | Blue);
		}
		Row += Buffer.Pitch;
	}
}

win32_window_dimension 
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
	Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
	
	RenderWeirdGradient(*Buffer, 128,0);
	
}

internal void
Win32DisplayBufferToWindow(HDC DeviceContext, win32_window_dimension WindowDim, win32_offscreen_buffer Buffer){
	
	StretchDIBits(DeviceContext, 
		0,0, WindowDim.Width, WindowDim.Height,
		0,0, Buffer.Width, Buffer.Height,
		Buffer.Memory,
		&Buffer.Info,
		DIB_RGB_COLORS, SRCCOPY
	);
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
			
			Win32DisplayBufferToWindow(DeviceContext, Dimension, GlobalBackBuffer);
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
			Result = DefWindowProc(Window, Message, WParam, LParam);
		} break;
	}
	
	return(Result);
}

int CALLBACK WinMain(HINSTANCE Instance,
					 HINSTANCE PrevInstance,
					 LPSTR CommandLine,
					 int ShowCode)
{
	WNDCLASS WindowClass = {};
	
	Win32ResizeDibSection(&GlobalBackBuffer, 1280, 720);
	
	WindowClass.style = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	WindowClass.lpfnWndProc = Win32MainWindowCallback;
	WindowClass.hInstance = Instance;
//						WindowClass.hIcon ;
	WindowClass.lpszClassName = "CosmoWindowClass";
	
	if(RegisterClass(&WindowClass))
	{
		HWND Window = CreateWindowExA(
		  0,
		  WindowClass.lpszClassName,
		  "Cosmo",
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
			GlobalRunning = true;
			int XOffset = 0;
			int YOffset = 0;
			
			while(GlobalRunning){
				
				MSG Message;
				while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
				{
					
					if (Message.message == WM_QUIT) {
						GlobalRunning = false;
					}
					TranslateMessage(&Message);
					DispatchMessageA(&Message);
				}
				
				RenderWeirdGradient(GlobalBackBuffer, XOffset, YOffset);

				
				win32_window_dimension Dimension = Win32GetWindowDimension(Window);
				Win32DisplayBufferToWindow(DeviceContext, Dimension, GlobalBackBuffer);
				
				++XOffset;
				++YOffset;
			}
			
		}
	}
	else{
	}
	
		
	 return(0);
 }