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
struct AppOffscreenBuffer
{
    // NOTE(jack): Pixels are alwasy 32-bits wide, Memory Order BB GG RR XX
    void *memory;
    int width;
    int height;
    int pitch;
    int bytes_per_pixel;
};

// TODO(jack): In the future, rendering _specifically_ will become a three-tiered abstraction!!!
struct AppInput
{
    f32 dt_for_frame;
};

struct AppMemory
{
    b32 is_initialised;
    
    u64 permanent_storage_size;
    void *permanent_storage; // NOTE(jack): REQUIRED to be cleared to zero at startup
    
    u64 transient_storage_size;;
    void *transient_storage; // NOTE(jack): REQUIRED to be cleared to zero at startup
    
    // NOTE(jack): Platfrom provided functions can be put here
    
};

typedef void AppUpdateFunc(AppMemory *, AppInput *, AppOffscreenBuffer *);

#ifndef USE_HOT_RELOAD
extern "C" void
app_update(AppMemory *Memory, AppInput *Input, AppOffscreenBuffer *Buffer);
#endif

#endif //PLATFROM_H
