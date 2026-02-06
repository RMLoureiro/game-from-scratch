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

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable int BitmapWidth;
global_variable int BitmapHeight;
global_variable int BytesPerPixel = 4;

internal void RenderWeirdGradient(int XOffset, int YOffset) {

    int Width = BitmapWidth;
    int Height = BitmapHeight;

    int Pitch = Width * BytesPerPixel;
        uint8 *Row = (uint8 *)BitmapMemory;
        for(int Y = 0; Y < BitmapHeight; ++Y) {
            uint32 *Pixel = (uint32 *)Row;

            for(int X = 0; X < BitmapWidth; ++X) {
                uint8 Blue = (X + XOffset);
                uint8 Green = (Y + YOffset);

                *Pixel++ = ((Green << 8) | Blue);
            }
            Row += Pitch;
        }
}

internal void Win64ResizeDIBSection(int Width, int Height){
    if(BitmapMemory) {
        VirtualFree(BitmapMemory, 0, MEM_RELEASE);
    }

    BitmapWidth = Width;
    BitmapHeight = Height;

    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32; // bword aligned
    BitmapInfo.bmiHeader.biCompression = BI_RGB;
    
    int BytesPerPixel = 4;
    int BitmapMemorySize = BytesPerPixel * BitmapWidth * BitmapHeight;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void Win64UpdateWindow(HDC DeviceContext, RECT *ClientRect, int X, int Y, int Width, int Height) {
    int WindowWidth = ClientRect->right - ClientRect->left;
    int WindowHeight = ClientRect->bottom - ClientRect->top;

    StretchDIBits(DeviceContext, 0, 0, BitmapWidth, BitmapHeight, 0, 0, WindowWidth, WindowHeight, BitmapMemory, &BitmapInfo, DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win64MainWindowCallback(
    HWND   Window,
    UINT   Message,
    WPARAM WParam,
    LPARAM LParam) 
{
    LRESULT Result = 0;

    switch(Message) {
        case WM_SIZE: {
            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            int Width = ClientRect.right - ClientRect.left;
            int Height = ClientRect.bottom - ClientRect.top;

            Win64ResizeDIBSection(Width, Height);
        } break;
        case WM_DESTROY: {
            Running = false;
        } break;
        case WM_CLOSE: {
            Running = false;
        } break;
        case WM_ACTIVATEAPP: {
            OutputDebugStringA("WM_ACTIVATEAPP\n");
        } break;
        case WM_PAINT: {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);
            int X = Paint.rcPaint.left;
            int Y = Paint.rcPaint.top;
            LONG Height = Paint.rcPaint.bottom - Paint.rcPaint.top;
            LONG Width = Paint.rcPaint.right - Paint.rcPaint.left;

            RECT ClientRect;
            GetClientRect(Window, &ClientRect);
            Win64UpdateWindow(DeviceContext, &ClientRect, X, Y, Width, Height);

            EndPaint(Window, &Paint);
        } break;

        default: {
            Result = DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    return (Result);
}

int CALLBACK WinMain(
    HINSTANCE Instance,
    HINSTANCE PrevInstance,
    LPSTR     CommandLine,
    int       ShowCode) {
    WNDCLASSA WindowClass = {};

    WindowClass.lpfnWndProc = Win64MainWindowCallback;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&WindowClass)){
        HWND Window = CreateWindowExA(
            0,
            WindowClass.lpszClassName,
            "Handmade Hero",
            WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            CW_USEDEFAULT,
            0,
            0,
            Instance,
            0);

        if (Window) {
            Running = true;
            int XOffset = 0;
            int YOffset = 0;

            while (Running) {
                MSG Message;
                while(BOOL MessageResult = PeekMessageA(&Message, 0, 0, 0, PM_REMOVE)) {
                    if(Message.message == WM_QUIT) {
                        Running = false;
                    }

                    TranslateMessage(&Message);
                    DispatchMessageA(&Message);
                }
                RenderWeirdGradient(XOffset, YOffset);
                HDC DeviceContext = GetDC(Window);

                RECT ClientRect;
                GetClientRect(Window, &ClientRect);
                int WindowWidth = ClientRect.right - ClientRect.left;
                int WindowHeight = ClientRect.bottom - ClientRect.top;
                Win64UpdateWindow(DeviceContext, &ClientRect, 0, 0, WindowWidth, WindowHeight);
                ReleaseDC(Window, DeviceContext);

                ++XOffset;
            }
        }

    }
    
    return (0);
}