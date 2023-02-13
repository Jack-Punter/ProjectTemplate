/* date = June 23rd 2022 5:56 pm */

#ifndef WIN32_PLATFORM_H
#define WIN32_PLATFORM_H

struct win32_app_code
{
    HMODULE AppCodeDLL;
    FILETIME DLLLastWriteTime;
    
    // IMPORTANT(jack): Either of the callbacks can be 0!  You must
    // check before calling.
    app_update_func *Update;
    
    b32 IsValid;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_offscreen_buffer
{
    // NOTE(jack): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct win32_state 
{
    u64 TotalSize;
    void *AppMemoryBlock;
    
    // TODO(jack): Use for storing applicatoin DLL path when moveing to dll based app
    char EXEFileName[MAX_PATH];
    char *OnePastLastEXEFileNameSlash;
};

#endif //WIN32_PLATFORM_H
