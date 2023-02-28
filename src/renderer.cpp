#include "renderer.h"

void
draw_rectangle(Renderer *renderer, RoundedRect rect, u32 colour)
{
    AppOffscreenBuffer *buffer = &renderer->buffer;
    f32 aspect = (f32)buffer->width / buffer->height;
    
    i32 start_x = rect.x;
    i32 end_x = rect.x + rect.width;
    i32 start_y = rect.y;
    i32 end_y = rect.y + rect.height;
    Assert(start_x >= 0 && end_x < buffer->width);
    Assert(start_y >= 0 && end_y < buffer->height);
    
    f32 center_x = (f32)(2.0f * ((start_x + end_x) / 2) - buffer->width) / buffer->height;
    f32 center_y = (f32)(2.0f * ((start_y + end_y) / 2) - buffer->height) / buffer->height;
    
    f32 norm_width = aspect * (f32)(end_x - start_x) / buffer->width;
    f32 norm_height = (f32)(end_y - start_y) / buffer->height;
    
    u32 *Pixels = (u32 *)buffer->memory;
    for (i32 j = start_y; j <= end_y; ++j)
    {
        u32 *Pixel = &Pixels[j * buffer->width + start_x];
        
        f32 py = (2.0f * j - (f32)buffer->height) / buffer->height;
        py -= center_y;
        
        for (i32 i = start_x; i <= end_x; ++i)
        {
            f32 px = (2.0f * i - (f32)buffer->width) / (f32)buffer->height;
            px -= center_x;
            
            f32 dp = sdRoundBox(px, py, norm_width, norm_height, rect.roundness);
            if (dp <= 0) {
                if (dp >= -0.005f) {
                    *Pixel = 0xFFFF0000;
                } else {
                    *Pixel = colour;
                }
            }
            Pixel++;
        }
    }
}
