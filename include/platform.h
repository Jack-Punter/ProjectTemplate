/* date = June 23rd 2022 5:34 pm */

#ifndef PLATFROM_H
#define PLATFROM_H

#include <stdint.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef i32 b32;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)


// TODO(jack): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct app_offscreen_buffer
{
    // NOTE(jack): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

// TODO(jack): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct app_input
{
    f32 dtForFrame;
};

struct app_memory
{
    b32 IsInitialized;
    
    u64 PermanentStorageSize;
    void *PermanentStorage; // NOTE(jack): REQUIRED to be cleared to zero at startup
    
    u64 TransientStorageSize;
    void *TransientStorage; // NOTE(jack): REQUIRED to be cleared to zero at startup
    
    // NOTE(jack): Platfrom provided functions can be put here
    
};

typedef void app_update_func(app_memory *Memory, app_input *Input, app_offscreen_buffer *Buffer);

extern "C" void
app_update(app_memory *Memory, app_input *Input, app_offscreen_buffer *Buffer);

#endif //PLATFROM_H
