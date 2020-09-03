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
            // Lower to 2 to dnot get the crosses on the sides
            if ((a.x != b.x) + (a.y != b.y) + (a.z != b.z) < 2)
                GFX::push_line(a, b, color, 0.01);
        }
    }
}

void draw_ray_hit(RayHit a, Vec4 color) {
    if (a) {
        GFX::push_point(a.point, color, 0.04);
        GFX::push_line(a.point, a.point + a.normal, color, 0.005);
        GFX::push_line(a.point, a.point + a.normal * a.t, color, 0.01);
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

RayHit collision_line_box(Vec3 origin, Vec3 dir, Box a) {
    bool inside = true;
    Vec3 min = a.position - a.half_size;
    Vec3 max = a.position + a.half_size;
    Vec3 point;
    Vec3 max_t = Vec3(-1, -1, -1);

    // Find candidate planes.
    for (int i = 0; i < 3; i++) {
        if (origin._[i] < min._[i])
            point._[i] = min._[i];
        else if (origin._[i] > max._[i])
            point._[i] = max._[i];
        else
            continue;
        if (dir._[i])
            max_t._[i] = (point._[i] - origin._[i]) / dir._[i];
        inside = false;
    }

    // Find Max T
    int plane = 0;
    if (max_t._[1] > max_t._[plane]) plane = 1;
    if (max_t._[2] > max_t._[plane]) plane = 2;

    // Ray origin inside bounding box
    if (inside)
        return { 0, origin };

    // Check final candidate actually inside box
    if (max_t._[plane] < 0)
        return { -1, origin };
    point = origin + max_t._[plane] * dir;
    for (int i = 0; i < 3; i++) {
        if (i == plane) continue;
        if (point._[i] < min._[i] || point._[i] > max._[i])
            return { -1, origin };
    }

    Vec3 normal = {};
    normal._[plane] = Math::sign(origin._[plane] - point._[plane]);
    return { max_t._[plane], point, normal };
}

RayHit continous_collision_check(Box a, Vec3 vel_a, Box b, Vec3 vel_b) {
    // Change the problem to something that is WAY EASIER to solve.
    Vec3 total_vel = vel_a - vel_b;
    Box extended_box = b.extend(a);

    draw_box(extended_box, Vec4(1.0, 0.4, 0.3, 1.0));
    GFX::push_line(a.position, total_vel, Vec4(1.0, 0.4, 0.3, 1.0));

    return collision_line_box(a.position, total_vel, b.extend(a));
}
