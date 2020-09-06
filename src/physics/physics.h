#pragma once
#include "../math/smek_vec.h"
#include <vector>

struct AABody {
    Vec3 position;
    Vec3 velocity;

    Vec3 half_size;

    real mass;
    real curr_t;

    void integrate_part(real t, real delta) {
        position += velocity * t * delta;
        curr_t += t;
        ASSERT_LT(curr_t, 1.0);
    }

    void integrate(real delta) {
        position += velocity * (1 - curr_t) * delta;
        curr_t = 0;
    }

    AABody extend(const Vec3 &ext) const {
        AABody copy = *this;
        copy.half_size += ext;
        return copy;
    }
};

struct RayHit {
    real t;
    real depth;
    Vec3 point;
    Vec3 normal;

    AABody *a, *b;

    operator bool() const {
        // We include 0, since collisions when the origin
        // is inside the box exists.
        return t >= 0 && t <= 1;
    }
};

struct Collision {
    Vec3 normal;
    f32 depth;

    operator bool() const {
        return depth > 0;
    }
};

struct PhysicsEngine {
    std::vector<AABody> bodies;

    void add_box(AABody b);
    void update(real delta);
    void draw();
};

void draw_box(AABody a, Vec4 color);

void draw_ray_hit(RayHit a, Vec4 color);

Collision collision_check(AABody a, AABody b);

RayHit collision_line_box(Vec3 start, Vec3 dir, AABody a);

bool check_and_solve_collision(AABody *a, AABody *b, real delta);

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
