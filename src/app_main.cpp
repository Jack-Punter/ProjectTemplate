#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <memory>

#include "platform.h"

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#define Assert(x) do { if (!(x)) { *(int *)0; } } while(0)

struct RoundedRect {
    f32 x, y, width, height;
    f32 roundness;
};

inline float
sdRoundBox(f32 u, f32 v, f32 r) {
    f32 qu = MAX(abs(u) - 1.0f + r, 0.0f);
    f32 qv = MAX(abs(v) - 1.0f + r, 0.0f);
    return MIN(MAX(qu, qv), 0.0f) + sqrt(qu * qu + qv * qv) - r;
}

void
draw_rectangle(app_offscreen_buffer *Buffer, RoundedRect rect, u32 colour)
{
    i32 start_x = (i32)(rect.x * Buffer->Width);
    i32 end_x = (i32)((rect.x + rect.width) * Buffer->Width);
    i32 start_y = (i32)(rect.y * Buffer->Height);
    i32 end_y = (i32)((rect.y + rect.height) * Buffer->Height);
    
    i32 pcenter_x = (start_x + end_x) / 2;
    i32 pcenter_y = (start_y + end_y) / 2;
    
    f32 center_x = rect.x + rect.width / 2.0f;
    f32 center_y = rect.y + rect.height / 2.0f;
    
    u32 *Pixels = (u32 *)Buffer->Memory;
    for (i32 j = start_y; j < end_y; ++j)
    {
        // u32 *Pixel = &Pixels[j * Buffer->Width];
        f32 py = (2.0f * (j) - (f32)Buffer->Height) / (f32)Buffer->Height;
        for (i32 i = start_x; i < end_x; ++i) {
            f32 px = (2.0f * (i) - (f32)Buffer->Width) / (f32)Buffer->Height;
            f32 qu = MAX(abs(px) - rect.width + rect.roundness, 0.0f);
            f32 qv = MAX(abs(py) - rect.height + rect.roundness, 0.0f);
            
            f32 dp = MIN(MAX(qu, qv), 0.0f) + sqrt(qu * qu + qv * qv) - rect.roundness;
            if (dp <= 0) {
                // Pixel[i] = colour;
                Pixels[j * Buffer->Width + i] = colour;
            }
        }
    }
}

extern "C" void
app_update(app_memory *Memory, app_input *Input, app_offscreen_buffer *Buffer)
{
    memset(Buffer->Memory, 0, Buffer->Width * Buffer->Height * Buffer->BytesPerPixel);
    static f32 timer = 0.0f;
    timer += Input->dtForFrame;
    if (timer > 2.0f * 3.141592f) {
        timer -= 2.0f * 3.141592f;
    }
    
    f32 x, y;
    x = 0.3f;
    y = 0.4f;
    f32 width = 0.4f;
    f32 height = 0.2f;
    
    f32 amplitude = 0.5f * MIN(width, height);
    f32 r = amplitude + amplitude * cos(timer);
    
    RoundedRect rectangle = {
        x,
        y,
        width,
        height,
        r,
    };
    
    i32 x1 = (i32)(x * Buffer->Width);
    i32 x2 = (i32)((x + width) * Buffer->Width);
    u32 *Pixels = (u32 *)Buffer->Memory;
    for (i32 j = 0; j < Buffer->Height; ++j) {
        Pixels[j * Buffer->Width + x1] = 0xFFFFFFFF;
        Pixels[j * Buffer->Width + x2] = 0xFFFFFFFF;
    }
    
    i32 y1 = (i32)(y * Buffer->Height);
    i32 y2 = (i32)((y + height) * Buffer->Height);
    for (i32 i = 0; i < Buffer->Width; ++i) {
        Pixels[y1 * Buffer->Width + i] = 0xFFFFFFFF;
        Pixels[y2 * Buffer->Width + i] = 0xFFFFFFFF;
    }
    
    // draw_rectangle(Buffer, rectangle, 0xFF987BBC);
    draw_rectangle(Buffer, rectangle, 0xFFFF00BC);
}
