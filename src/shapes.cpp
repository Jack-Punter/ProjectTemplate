#include "shapes.h"

inline f32
sdRoundBox(f32 u, f32 v, f32 width, f32 height, f32 r) {
    f32 qu = abs(u) - width + r;
    f32 qv = abs(v) - height + r;
    f32 dx = MAX(qu, 0.0f);
    f32 dy = MAX(qv, 0.0f);
    f32 dp = MIN(MAX(qu, qv), 0.0f) + sqrt(dx * dx + dy * dy) - r;
    return dp;
}
