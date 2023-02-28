#include <stdio.h>
#include <stdlib.h>
#include <cmath>
#include <memory>

#define Assert(x) do { if (!(x)) { *(int *)0; } } while(0)

#define MIN(x, y) ((x) < (y) ? (x) : (y))
#define MAX(x, y) ((x) > (y) ? (x) : (y))

#include "platform.h"

#include "shapes.h"
#include "renderer.h"

#include "shapes.cpp"
#include "renderer.cpp"

extern "C" void
app_update(AppMemory *memory, AppInput *input, AppOffscreenBuffer *buffer)
{
    memset(buffer->memory, 0, buffer->width * buffer->height * buffer->bytes_per_pixel);
    static f32 timer = 0.0f;
    timer += input->dt_for_frame;
    if (timer > 2.0f * 3.141592f) {
        timer -= 2.0f * 3.141592f;
    }
    
    Renderer renderer = { *buffer };
    
    f32 x, y;
    x = 0.1f;
    y = 0.1f;
    f32 width = 0.5f;
    f32 height = 0.5f;
    
    f32 amplitude = 0.5f * MIN(width, height);
    f32 r = MAX(amplitude + amplitude * cos(timer), 0.0f);
    
    RoundedRect rect= {
        (i32)(x * buffer->width),
        (i32)(y * buffer->height),
        (i32)(width * buffer->width),
        (i32)(height * buffer->height),
        r,
    };
    
    u32 *Pixels = (u32 *)buffer->memory;
    for (i32 j = 0; j < buffer->height; ++j) {
        Pixels[j * buffer->width + rect.x] = 0xFFFFFFFF;
        Pixels[j * buffer->width + rect.x + rect.width] = 0xFFFFFFFF;
    }
    
    for (i32 i = 0; i < buffer->width; ++i) {
        Pixels[rect.y * buffer->width + i] = 0xFFFFFFFF;
        Pixels[(rect.y + rect.height) * buffer->width + i] = 0xFFFFFFFF;
    }
    
    draw_rectangle(&renderer, rect, 0xFF987BBC);
    // draw_rectangle(Buffer, rect, 0xFFFF00BC);
}
