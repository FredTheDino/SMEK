#pragma once
#include "../math/smek_vec.h"
#include <vector>

///* AABody
// An axis aligned body that
// is always shaped like a box.
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

struct Manifold {
    real t;
    real depth;
    Vec3 point;
    Vec3 normal;

    AABody *a, *b;

    operator bool() const {
        // We include 0, since collisions when the origin
        // is inside the box exists.
        return (t > 0 && t <= 1) || (depth >= 0 && t >= 0);
    }
};

///* PhysicsEngine
// A struct that handles all the collisions and
// the bodies of a world.
struct PhysicsEngine {
    std::vector<AABody> bodies;

    void add_box(AABody b);
    void update(real delta);
    void draw();
};

///* draw_aabody
// Draws a body as a box.
void draw_aabody(AABody a, Vec4 color);

///* draw_manifold
// Draws a collision manifold as a position and a normal.
// Position might not match "real" position.
void draw_manifold(Manifold a, Vec4 color);

///* collision_aabb
// Check the collisions between two bodies.
Manifold collision_aabb(AABody *a, AABody *b);

///* collision_line_box
// Check the collisions between a line and a box.
Manifold collision_line_box(Vec3 start, Vec3 dir, AABody a);

#include "../test.h"

TEST_CASE("collision test simple", {
    AABody a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return collision_aabb(&a, &b);
});

TEST_CASE("collision test simple", {
    AABody a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(0, 0, 0);

    AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 0, 0);

    return !collision_aabb(&a, &b);
});

TEST_CASE("collision test simple", {
    AABody a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 1, 1);

    return collision_aabb(&a, &b);
});

TEST_CASE("collision test simple", {
    AABody a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    AABody b;
    b.position = Vec3(1, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return !_close_enough_vec(collision_aabb(&b, &a).normal, collision_aabb(&a, &b).normal, 0.1);
});
