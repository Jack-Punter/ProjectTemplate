#ifndef SHAPES_H
#define SHAPES_H

//- Rectangles 

struct RoundedRect {
    i32 x, y, width, height;
    f32 roundness;
};

f32 
sdRoundBox(f32 u, f32 v, f32 width, f32 height, f32 r);

#endif //SHAPES_H
