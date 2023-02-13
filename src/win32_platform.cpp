#define NOMINMAX
#include <windows.h>
#include <Windows.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef AUTO_TRACING
#include "spall/spall_auto.h"
#endif

#include "platform.h" 
#include "win32_platform.h"

#define USE_HOT_RELOAD 1

#define internal static
#define global static

global b32 GlobalRunning = true;
global win32_offscreen_buffer GlobalBackbuffer;
global i64 GlobalPerfCountFrequency;

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO(jack): Dest bounds checking!
    
    for(int Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    
    for(int Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }
    
    *Dest++ = 0;
}

internal void
Win32GetEXEFileName(win32_state *State)
{
    // NOTE(jack): Never use MAX_PATH in code that is user-facing, because it
    // can be dangerous and lead to bad results.
    DWORD SizeOfFilename = GetModuleFileNameA(0, State->EXEFileName, sizeof(State->EXEFileName));
    State->OnePastLastEXEFileNameSlash = State->EXEFileName;
    for(char *Scan = State->EXEFileName;
        *Scan;
        ++Scan)
    {
        if(*Scan == '\\')
        {
            State->OnePastLastEXEFileNameSlash = Scan + 1;
        }
    }
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

internal void
Win32BuildEXEPathFileName(win32_state *State, char *FileName,
                          int DestCount, char *Dest)
{
    CatStrings(State->OnePastLastEXEFileNameSlash - State->EXEFileName, State->EXEFileName,
               StringLength(FileName), FileName,
               DestCount, Dest);
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;
    
    return(Result);
}

inline f32
Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    f32 Result = ((f32)(End.QuadPart - Start.QuadPart) /
                  (f32)GlobalPerfCountFrequency);
    return(Result);
}

inline LARGE_INTEGER
Win32GetWallClock(void)
{    
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return(Result);
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{
    // TODO(jack): Bulletproof this.
    // Maybe don't free first, free after, then free first if that fails.
    
    if(Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }
    
    Buffer->Width = Width;
    Buffer->Height = Height;
    
    int BytesPerPixel = 4;
    Buffer->BytesPerPixel = BytesPerPixel;
    
    // NOTE(jack): When the biHeight field is negative, this is the clue to
    // Windows to treat this bitmap as top-down, not bottom-up, meaning that
    // the first three bytes of the image are the color for the top left pixel
    // in the bitmap, not the bottom left!
    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;
    
    // NOTE(jack): Thank you to Chris Hecker of Spy Party fame
    // for clarifying the deal with StretchDIBits and BitBlt!
    // No more DC for us.
    int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    Buffer->Pitch = Width*BytesPerPixel;
    
    // TODO(jack): Probably clear this to black
}


internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer,
                           HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    // TODO(jack): Centering / black bars?
    
    if((WindowWidth >= Buffer->Width*2) &&
       (WindowHeight >= Buffer->Height*2))
    {
        StretchDIBits(DeviceContext,
                      0, 0, 2*Buffer->Width, 2*Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
    else
    {
#if 0
        int OffsetX = 10;
        int OffsetY = 10;
        
        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DeviceContext, 0, OffsetY + Buffer->Height, WindowWidth, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, OffsetX + Buffer->Width, 0, WindowWidth, WindowHeight, BLACKNESS);
        
        // NOTE(jack): For prototyping purposes, we're going to always blit
        // 1-to-1 pixels to make sure we don't introduce artifacts with
        // stretching while we are learning to code the renderer!
        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
#endif
        
        // PatBlt(DeviceContext, 0, 0, WindowWidth, WindowHeight, BLACKNESS);
        StretchDIBits(DeviceContext,
                      0, 0, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}
inline FILETIME
Win32GetLastWriteTime(char *Filename)
{
    FILETIME LastWriteTime = {};
    
    WIN32_FILE_ATTRIBUTE_DATA Data;
    if(GetFileAttributesEx(Filename, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    
    return(LastWriteTime);
}

#if USE_HOT_RELOAD
internal win32_app_code
Win32LoadAppCode(char *SourceDLLName, char *TempDLLName)
{
    win32_app_code Result = {};
    
    Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceDLLName);
    
    CopyFile(SourceDLLName, TempDLLName, FALSE);
    
    Result.AppCodeDLL = LoadLibraryA(TempDLLName);
    if(Result.AppCodeDLL)
    {
        Result.Update = (app_update_func *)
            GetProcAddress(Result.AppCodeDLL, "app_update");
        
        Result.IsValid = (Result.Update != 0);
    }
    
    if(!Result.IsValid)
    {
        Result.Update = 0;
    }
    
    return(Result);
}

internal void
Win32UnloadAppCode(win32_app_code *AppCode)
{
    if(AppCode->AppCodeDLL)
    {
        FreeLibrary(AppCode->AppCodeDLL);
        AppCode->AppCodeDLL = 0;
    }
    
    AppCode->IsValid = false;
    AppCode->Update = 0;
}
#endif

LRESULT CALLBACK MessageCallback(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam)
{
    switch (Message)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC DeviceContext = BeginPaint(Window, &ps);
            
            win32_window_dimension Dimension = Win32GetWindowDimension(Window);
            Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                       Dimension.Width, Dimension.Height);
            
            EndPaint(Window, &ps);
        } break;
        
        case WM_SIZE:
        {
            Win32ResizeDIBSection(&GlobalBackbuffer, LOWORD(LParam), HIWORD(LParam));
        } break;
        
        case WM_CLOSE:
        {
#if 0
            if (MessageBox(Window, "Really quit?", "My application", MB_OKCANCEL) == IDOK)
            {
                GlobalRunning = false;
                DestroyWindow(Window);
            }
#endif
            GlobalRunning = false;
            // Else: User canceled. Do nothing.
            return 0;
        }
        
        case WM_ACTIVATEAPP:
        {
#if 1
            if(WParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            }
            else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 64, LWA_ALPHA);
            }
#endif
        } break;
        
        default: 
        {
            return DefWindowProc(Window, Message, WParam, LParam);
        } break;
    }
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
#ifdef AUTO_TRACING
    char spall_file[] = "_prj_template.spall";
    spall_auto_init(spall_file);
    spall_auto_thread_init(0, SPALL_DEFAULT_BUFFER_SIZE, SPALL_DEFAULT_SYMBOL_CACHE_SIZE);
#endif
    
    printf("Entering main\n");
    //-
    // NOTE(jack): Init window class
    WNDCLASSEX wndClass;
    wndClass.cbSize = sizeof(wndClass);
    wndClass.style = CS_HREDRAW | CS_VREDRAW;
    wndClass.lpfnWndProc = MessageCallback;
    wndClass.cbClsExtra = 0; // NO extra class mem
    wndClass.cbWndExtra = 0; // No extra window memory
    wndClass.hInstance = hInstance; // handle to instance 
    wndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION); // predefined app. icon 
    wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);// predefined arrow 
    wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH); // Black background brush 
    wndClass.lpszMenuName =  "MainMenu"; // name of menu resource 
    wndClass.lpszClassName = "MainWClass"; // name of window class 
    wndClass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    
    if (!RegisterClassEx(&wndClass)) {
        printf("Failed to register window class\n");
        return false;
    }
    
    //-
    // NOTE(jack): Create Window Instance
    
    HWND Window = CreateWindowEx(WS_EX_LAYERED | WS_EX_TOPMOST,
                                 "MainWClass", // Name of class
                                 "app_main", // Title-bar string
                                 // NOTE(jack): Allows window to stay on top, but be grayed out when not focused
                                 WS_OVERLAPPEDWINDOW|WS_VISIBLE, // top-level window
                                 CW_USEDEFAULT, // default horizontal position
                                 CW_USEDEFAULT, // default vertical position
                                 1280, // width
                                 720, // height)
                                 0, // no owner window
                                 0, // use class menu
                                 hInstance, // handle to application instance
                                 0); // no window-creation data
    if (!Window) {
        printf("Failed to create window\n");
        DWORD last_error = GetLastError();
        return false;
    }
    
    int MonitorRefreshHz = 60;
    HDC RefreshDC = GetDC(Window);
    int Win32RefreshRate = GetDeviceCaps(RefreshDC, VREFRESH);
    ReleaseDC(Window, RefreshDC);
    if(Win32RefreshRate > 1)
    {
        MonitorRefreshHz = Win32RefreshRate;
    }
    f32 GameUpdateHz = (MonitorRefreshHz / 2.0f);
    f32 TargetSecondsPerFrame = 1.0f / (f32)GameUpdateHz;
    // NOTE(jack): Set the Windows scheduler granularity to 1ms
    // so that our Sleep() can be more granular.
    UINT DesiredSchedulerMS = 1;
    b32 SleepIsGranular = (timeBeginPeriod(DesiredSchedulerMS) == TIMERR_NOERROR);
    
    // NOTE(jack): Show the window and send a WM_PAINT message to the window proc
    ShowWindow(Window, nCmdShow);
    UpdateWindow(Window);
    
    
    //-
    // NOTE(jack): Message pump
    Win32ResizeDIBSection(&GlobalBackbuffer, 1280, 720);
    LARGE_INTEGER LastCounter = Win32GetWallClock();
    
    LARGE_INTEGER PerfCountFrequencyResult;
    QueryPerformanceFrequency(&PerfCountFrequencyResult);
    GlobalPerfCountFrequency = PerfCountFrequencyResult.QuadPart;
    
    app_input Input[2] = {};
    app_input *OldInput = &Input[0];
    app_input *NewInput = &Input[1];
    
    app_memory AppMemory= {};
    AppMemory.PermanentStorageSize = Megabytes(256);
    AppMemory.TransientStorageSize = Gigabytes(1);
    
    win32_state Win32State = {};
    Win32GetEXEFileName(&Win32State);
    
    char SourceAppCodeDLLFullPath[MAX_PATH];
    Win32BuildEXEPathFileName(&Win32State, (char*)"app_main.dll",
                              sizeof(SourceAppCodeDLLFullPath), SourceAppCodeDLLFullPath);
    
    char TempAppCodeDLLFullPath[MAX_PATH];
    Win32BuildEXEPathFileName(&Win32State, (char*)"app_main_temp.dll",
                              sizeof(TempAppCodeDLLFullPath), TempAppCodeDLLFullPath);
    
    char AppCodeLockFullPath[MAX_PATH];
    Win32BuildEXEPathFileName(&Win32State, (char*)"lock.tmp",
                              sizeof(AppCodeLockFullPath), AppCodeLockFullPath);
    
    win32_app_code Application = Win32LoadAppCode(SourceAppCodeDLLFullPath, TempAppCodeDLLFullPath);
    
    Win32State.TotalSize = AppMemory.PermanentStorageSize + AppMemory.TransientStorageSize;
    Win32State.AppMemoryBlock = VirtualAlloc(0, (size_t)Win32State.TotalSize,
                                             MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);
    
    AppMemory.PermanentStorage = Win32State.AppMemoryBlock;
    AppMemory.TransientStorage = ((u8 *)AppMemory.PermanentStorage +
                                  AppMemory.PermanentStorageSize);
    
    while (GlobalRunning)
    {
        
#if USE_HOT_RELOAD
        FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceAppCodeDLLFullPath);
        WIN32_FILE_ATTRIBUTE_DATA Ignored;
        // NOTE(jack): GetFileAttributesEx Returns 0 if it failed, which means the lock was not present
        if(CompareFileTime(&NewDLLWriteTime, &Application.DLLLastWriteTime) != 0 &&
           !GetFileAttributesEx(AppCodeLockFullPath, GetFileExInfoStandard, &Ignored))
        {
            Win32UnloadAppCode(&Application);
            Application = Win32LoadAppCode(SourceAppCodeDLLFullPath,
                                           TempAppCodeDLLFullPath);
        }
#endif
        
        
        MSG Message;
        while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        
        HDC DeviceContext = GetDC(Window);
        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
        Win32DisplayBufferInWindow(&GlobalBackbuffer, DeviceContext,
                                   Dimension.Width, Dimension.Height);
        ReleaseDC(Window, DeviceContext);
        
        
        LARGE_INTEGER EndCounter = Win32GetWallClock();
        f32 SPerFrame = Win32GetSecondsElapsed(LastCounter, EndCounter);                    
        LastCounter = EndCounter;
        NewInput->dtForFrame = SPerFrame;
        
        char Title[256] = {};
        snprintf(Title, 256, "RayTracing (%.3f Fps, %.3f ms/f)", 1.0f / SPerFrame, 1000 * SPerFrame);
        SetWindowText(Window, Title);
        
        
        app_offscreen_buffer Buffer = {};
        Buffer.Memory = GlobalBackbuffer.Memory;
        Buffer.Width = GlobalBackbuffer.Width; 
        Buffer.Height = GlobalBackbuffer.Height;
        Buffer.Pitch = GlobalBackbuffer.Pitch;
        Buffer.BytesPerPixel = GlobalBackbuffer.BytesPerPixel;
        
#if USE_HOT_RELOAD
        if (Application.Update) {
            Application.Update(&AppMemory, NewInput, &Buffer);
        }
#else
        app_update(&AppMemory, NewInput, &Buffer);
#endif
        
        app_input *TmpInput = OldInput;
        OldInput = NewInput;
        NewInput = TmpInput;
        //- Wait for next frame 
        LARGE_INTEGER WorkCounter = Win32GetWallClock();
        f32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);
        
        // TODO(jack): NOT TESTED YET!  PROBABLY BUGGY!!!!!
        f32 SecondsElapsedForFrame = WorkSecondsElapsed;
        if(SecondsElapsedForFrame < TargetSecondsPerFrame)
        {                        
            if(SleepIsGranular)
            {
                DWORD SleepMS = (DWORD)(1000.0f * (TargetSecondsPerFrame -
                                                   SecondsElapsedForFrame));
                if(SleepMS > 0)
                {
                    Sleep(SleepMS);
                }
            }
            
            f32 TestSecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                    Win32GetWallClock());
            if(TestSecondsElapsedForFrame < TargetSecondsPerFrame)
            {
                // TODO(jack): LOG MISSED SLEEP HERE
            }
            
            while(SecondsElapsedForFrame < TargetSecondsPerFrame)
            {                            
                SecondsElapsedForFrame = Win32GetSecondsElapsed(LastCounter,
                                                                Win32GetWallClock());
            }
        }
        else
        {
            // TODO(jack): MISSED FRAME RATE!
            // TODO(jack): Logging
        }
    }
    printf("Normal Return\n");
    
#ifdef AUTO_TRACING
    spall_auto_thread_quit();
	spall_auto_quit();
#endif
    
    return 0;
}

#ifdef AUTO_TRACING
#define SPALL_AUTO_IMPLEMENTATION
#include "spall/spall_auto.h"
#endif
