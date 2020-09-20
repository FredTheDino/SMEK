#pragma once
#include "../math/smek_vec.h"
#include "../math/smek_quat.h"
#include <vector>

namespace Physics {

enum ShapeKind {
    BOX,
    SPHERE,

    NUM_SUPPORT_FUNC,
};

struct PhysicsShape {
    Vec3 position;
    Vec3 scale;
    Quat rotation;

    ShapeKind kind;

    Vec3 support(Vec3 d);
};

///* AABody
// An axis aligned body that
// is always shaped like a box.
struct AABody {
    Vec3 position;
    Vec3 velocity;

    Vec3 half_size;

    real mass;
    real curr_t;

    ///* integrate_part
    // Integrates part of a whole step.
    void integrate_part(real t, real delta);

    ///* integrate
    // Integrates the rest of the time step.
    void integrate(real delta);

    ///* extend
    // Extends the domain of this AABody
    // with the vector passed in.
    AABody extend(const Vec3 &ext) const;
};

struct MinSumResult {
    Vec3 normal;
    Vec3 point;
    f32 t;
};

MinSumResult minsum_ray(PhysicsShape *a, PhysicsShape *b, Vec3 dir);

///* Manifold
// An object with information about the collision.
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

///* collision_line_aabody
// Check the collisions between a line and a box.
Manifold collision_line_aabody(Vec3 start, Vec3 dir, AABody *a);

}

#include "../test.h"

TEST_CASE("collision simple", {
    Physics::AABody a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Physics::AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return collision_aabb(&a, &b);
});

TEST_CASE("collision simple", {
    Physics::AABody a;
    a.position = Vec3(1, 0, 0);
    a.half_size = Vec3(0, 0, 0);

    Physics::AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 0, 0);

    return !collision_aabb(&a, &b);
});

TEST_CASE("collision simple", {
    Physics::AABody a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Physics::AABody b;
    b.position = Vec3(0, 0, 0);
    b.half_size = Vec3(0, 1, 1);

    return collision_aabb(&a, &b);
});

TEST_CASE("collision simple", {
    Physics::AABody a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Physics::AABody b;
    b.position = Vec3(1, 0, 0);
    b.half_size = Vec3(1, 1, 1);

    return !_close_enough_vec(collision_aabb(&b, &a).normal, collision_aabb(&a, &b).normal, 0.1);
});

TEST_CASE("collision cast simple", {
    Physics::AABody a;
    a.position = Vec3(0, 0, 0);
    a.half_size = Vec3(1, 1, 1);

    Vec3 start = Vec3(0, -2, 0);
    Vec3 dir = Vec3(0, 1, 0);

    Physics::Manifold m = collision_line_aabody(start, dir, &a);
    return 0.9 < m.t && m.t < 1.1;
});
