#pragma once
#include "../math/smek_vec.h"

struct RayHit {
    real t;
    Vec3 point;
    Vec3 normal;

    operator bool() const {
        // We include 0, since collisions when the origin
        // is inside the box exists.
        return t >= 0 && t <= 1;
    }
};

struct Box {
    Vec3 position;
    Vec3 half_size;

    Box extend(const Box &b) const {
        return { position, half_size + b.half_size };
    }
};

struct Collision {
    Box a, b;
    Vec3 normal;
    f32 depth;

    operator bool() const {
        return depth > 0;
    }
};

void draw_box(Box a, Vec4 color);

void draw_ray_hit(RayHit a, Vec4 color);

Collision collision_check(Box a, Box b);

RayHit continous_collision_check(Box a, Vec3 vel_a, Box b, Vec3 vel_b);

RayHit collision_line_box(Vec3 start, Vec3 dir, Box a);

#include "../test.h"

TEST_CASE("collision test simple", {
    Box a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Box b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return collision_check(a, b);
});

TEST_CASE("collision test simple", {
    Box a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(0, 0, 0);

    Box b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 0, 0);

    return !collision_check(a, b);
});

TEST_CASE("collision test simple", {
    Box a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Box b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 1, 1);

    return collision_check(a, b);
});

TEST_CASE("collision test simple", {
    Box a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Box b;
    b.position = Vec3(1, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return !_close_enough_vec(collision_check(b, a).normal, collision_check(a, b).normal, 0.1);
});
