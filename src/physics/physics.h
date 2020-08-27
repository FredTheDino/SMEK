#pragma once
#include "../math/smek_vec.h"

struct Box {
    Vec3 position;
    Vec3 half_size;
};

struct Collision {
    Box a, b;
    Vec3 normal;
    f32 depth;

    operator bool() const {
        return depth > 0;
    }
};

Collision collision_check(Box a, Box b);

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
