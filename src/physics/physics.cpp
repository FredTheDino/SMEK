#include "physics.h"
#include "../renderer/renderer.h"

i32 format(char *buffer, u32 size, FormatHint args, Collision c) {
    return snprintf(buffer, size, "%0*.*f, (%0*.*f, %0*.*f, %0*.*f)",
                    args.num_zero_pad, args.num_decimals, c.depth,
                    args.num_zero_pad, args.num_decimals, c.normal.x,
                    args.num_zero_pad, args.num_decimals, c.normal.y,
                    args.num_zero_pad, args.num_decimals, c.normal.z);
}

void draw_box(Box a, Vec4 color) {
    Vec3 p = a.position;
    Vec3 r = a.half_size;
    Vec3 points[] = {
        p + Vec3(+r.x, +r.y, +r.z),
        p + Vec3(-r.x, +r.y, +r.z),
        p + Vec3(+r.x, -r.y, +r.z),
        p + Vec3(-r.x, -r.y, +r.z),
        p + Vec3(+r.x, +r.y, -r.z),
        p + Vec3(-r.x, +r.y, -r.z),
        p + Vec3(+r.x, -r.y, -r.z),
        p + Vec3(-r.x, -r.y, -r.z),
    };

    for (u32 i = 0; i < LEN(points); i++) {
        for (u32 j = i + 1; j < LEN(points); j++) {
            Vec3 a = points[i];
            Vec3 b = points[j];
            GFX::push_line(a, b, color);
        }
    }
}

Collision collision_check(Box a, Box b) {
    Vec3 delta = a.position - b.position;
    Vec3 range = a.half_size + b.half_size;
    Vec3 depths = range - abs(delta);

    Collision col = {};
    col.a = a;
    col.b = b;
    col.normal = Vec3(0, 0, 0);

    if (depths.x < depths.y && depths.x < depths.z) {
        col.normal.x = Math::sign(delta.x);
        col.depth = depths.x;
    } else if (depths.y < depths.x && depths.y < depths.z) {
        col.normal.y = Math::sign(delta.y);
        col.depth = depths.y;
    } else {
        col.normal.z = Math::sign(delta.z);
        col.depth = depths.z;
    }

    return col;
}