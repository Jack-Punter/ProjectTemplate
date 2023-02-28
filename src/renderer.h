#ifndef RENDERER_H
#define RENDERER_H

#include "shapes.h"

struct Renderer { 
    AppOffscreenBuffer buffer;
};

void draw_rectangle(Renderer *renderer, RoundedRect rect, u32 color);

#endif //RENDERER_H
