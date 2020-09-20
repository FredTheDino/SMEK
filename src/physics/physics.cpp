#include "physics.h"
#include "../renderer/renderer.h"
#include "imgui/imgui.h"

namespace Physics {

Vec3 box_support_func(Vec3 d) {
    using Math::sign;
    return Vec3(sign(d.x), sign(d.y), sign(d.z)) * 0.5;
}

Vec3 sphere_support_func(Vec3 d) {
    return normalized(d);
}

using SupportFunc = Vec3 (*)(Vec3 d);
SupportFunc SUPPORT_FUNCS[] = {
    [ShapeKind::BOX] = box_support_func,
    [ShapeKind::SPHERE] = sphere_support_func,
};

Vec3 PhysicsShape::support(Vec3 d) {
    Vec3 object_space = component_div((-rotation) * d, scale);
    Vec3 projected = SUPPORT_FUNCS[kind](object_space);
    return rotation * hadamard(scale, projected);
}

f32 triangle_ray_hit(bool *hit, Vec3 a, Vec3 b, Vec3 c, Vec3 origin, Vec3 dir) {
    *hit = false;
    const f32 EPSILON = 0.0001;
    const f32 NO_HIT = -1;
    Vec3 edge_ab = b - a;
    Vec3 edge_ac = c - a;

    // Möller-Trumbore intersection.
    //  - Move to triangle coordinate space to check for collisons.

    Vec3 h = cross(dir, edge_ac);
    f32 q = dot(edge_ab, h);
    if (q > EPSILON && q < EPSILON)
        return NO_HIT;

    Vec3 j = origin - a;
    f32 r = 1.0 / q;
    f32 s = r * dot(j, h);
    if (s < 0.0 || s > 1.0)
        return NO_HIT;

    Vec3 k = cross(j, edge_ab);
    f32 u = r * dot(dir, k);
    if (u < 0.0 || s + u > 1.0)
        return NO_HIT;

    *hit = true;
    return r * dot(edge_ac, k);
}

struct Simplex {
    u32 num_points = 0;
    Vec3 points[3];

    void add(Vec3 p, Vec3 origin, Vec3 dir) {
        u32 worst = num_points;
        if (num_points == LEN(points) - 1) {
            worst = 10;
            f32 worst_cost = 0;
            for (u32 i = 0; i < num_points; i++) {
                // We want to find points which are closest to
                // the source and the line. This is just some form
                // of heuristic for it, which should work pretty decent.
                f32 cost = dot(points[i] - origin, dir);
                // f32 cost = length_squared(t * dir + origin - points[i]) + t;
                if (cost > worst_cost || worst == 10) {
                    worst = i;
                    worst_cost = cost;
                }
            }
        } else {
            num_points++;
        }
        points[worst] = p;
    }

    bool possible(Vec3 origin, Vec3 dir) {
        for (u32 i = 0; i < num_points; i++) {
            f32 t = dot(points[i] - origin, dir);
            LOG("{} = ({} - {}) * {}", t, points[i], origin, dir);
            if (0.0 < t && t < 1.0) {
                return true;
            }
        }
        return false;
    }

    Vec3 next_search_dir(Vec3 dir, Vec3 origin) {
        if (num_points == 0)
            return -dir;

        if (num_points == 1)
            return origin - points[0];

        if (num_points == 2)
            return origin - points[0] + points[1];

        if (num_points == 3) {
            Vec3 normal = cross(points[1] - points[0], points[2] - points[0]);
            if (dot(dir, normal) < 0.0) {
                return normal;
            } else {
                return -normal;
            }
        }
        return dir;
    }

    f32 hit(bool *hit, Vec3 dir, Vec3 origin) {
        *hit = false;
        if (num_points != 3) return 0;
        return triangle_ray_hit(hit, points[0], points[1], points[2], origin, dir);
    }
};

MinSumResult minsum_ray(PhysicsShape *a, PhysicsShape *b, Vec3 dir) {
    MinSumResult result = {};

    Vec3 origin = b->position;
    Vec3 position = a->position;

    auto query = [&position, a, b](Vec3 d) -> Vec3 {
        return position + a->support(d) + b->support(d);
    };

    Simplex simplex;
    for (i32 loops = 0; loops < 10; loops++) {
        LOG("{}", loops);
        Vec3 search_dir = simplex.next_search_dir(origin, dir);
        Vec3 p = query(search_dir);
        GFX::push_point(p, Vec4(0.3, 0.7, 0.3, 1.0), 0.07);
        simplex.add(p, origin, dir);
        if (!simplex.possible(origin, dir)) {
            LOG("Not possible!");
            break;
        }
        bool hit = false;
        result.t = simplex.hit(&hit, origin, dir);
        if (hit) {
            LOG("hit: {}", result.t);
            break;
        }
    }

    {
        // Draws what the computer can see...
        f32 resolution = 0.03;
        for (f32 ad = 0; ad < 2 * PI; ad += resolution) {
            for (f32 bd = 0; bd < 2 * PI; bd += resolution) {
                using Math::cos;
                using Math::sin;
                Vec3 d = Mat::rotate(0, ad, bd) * Vec3(1, 0, 0);
                Vec3 res = a->support(d) + b->support(d);
                Vec4 color = Vec4(sin(res.x) * 0.5 + 0.5, cos(res.y) * 0.5 + 0.5, 0.3, 1.0);
                GFX::push_point(res + a->position, color, 0.05);
            }
        }
    }

    return result;
}

void draw_aabody(AABody a, Vec4 color) {
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
            // Lower to 2 to not get the crosses on the sides
            if ((a.x != b.x) + (a.y != b.y) + (a.z != b.z) < 2)
                GFX::push_line(a, b, color, 0.01);
        }
    }
}

void draw_manifold(Manifold a, Vec4 color) {
    if (a) {
        GFX::push_point(a.point, color, 0.04);
        GFX::push_line(a.point, a.point + a.normal, color, 0.005);
        GFX::push_line(a.point, a.point + a.normal * a.t, color, 0.01);
    }
}

void AABody::integrate_part(real t, real delta) {
    position += velocity * t * delta;
    curr_t += t;
    ASSERT_LT(curr_t, 1.0);
}

void AABody::integrate(real delta) {
    position += velocity * (1 - curr_t) * delta;
    curr_t = 0;
}

AABody AABody::extend(const Vec3 &ext) const {
    AABody copy = *this;
    copy.half_size += ext;
    return copy;
}

Manifold collision_aabb(AABody *a, AABody *b) {
    Vec3 delta = a->position - b->position;
    Vec3 range = a->half_size + b->half_size;
    Vec3 depths = range - abs(delta);

    Manifold col = {};
    col.point = (a->position + b->position) / 2.0;

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

Manifold collision_line_aabody(Vec3 origin, Vec3 dir, AABody *a) {
    bool inside = true;
    Vec3 min = a->position - a->half_size;
    Vec3 max = a->position + a->half_size;
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

    // Ray origin inside bounding box
    //
    // NOTE(ed): Not returning a normal here
    // makes things more numerically unstable.
    // I didn't mange to think of a way to do this,
    // but there might be a smart way to do it here.
    // Otherwise we fall back on the "collision_aabb"
    // function for the "t" since we know they are overlapping.
    if (inside)
        return { 0, 1.0, origin };

    // Find Max T
    int plane = 0;
    if (max_t._[1] > max_t._[plane]) plane = 1;
    if (max_t._[2] > max_t._[plane]) plane = 2;

    // Check final candidate actually inside box
    if (max_t._[plane] < 0)
        return { -1 };
    point = origin + max_t._[plane] * dir;
    for (int i = 0; i < 3; i++) {
        if (i == plane) continue;
        if (point._[i] < min._[i] || point._[i] > max._[i])
            return { -1 };
    }

    Vec3 normal = {};
    normal._[plane] = Math::sign(origin._[plane] - point._[plane]);
    return { max_t._[plane], 0.0, point, normal };
}

Manifold check_collision(AABody *a, AABody *b, real delta) {
    AABody extended_box = b->extend(a->half_size);

    Vec3 total_movement = (a->velocity - b->velocity) * delta;
    Manifold hit = collision_line_aabody(a->position, total_movement, &extended_box);
    if (hit.depth) {
        hit = collision_aabb(a, b);
    }
    hit.a = a;
    hit.b = b;
    return hit;
}

const real MARGIN = 0.001;
void solve_collision(Manifold hit, real delta) {
    AABody *a, *b;
    a = hit.a;
    b = hit.b;

    // Update velocities
    const real BOUNCE = 0.0;
    real vel_rel_norm = (1.0 + BOUNCE) * (dot(a->velocity, hit.normal) - dot(b->velocity, hit.normal));
    real total_mass = a->mass + b->mass;
    if (total_mass != 0 && vel_rel_norm < 0) {
        a->velocity += hit.normal * (-vel_rel_norm * a->mass / total_mass);
        b->velocity += hit.normal * (+vel_rel_norm * b->mass / total_mass);
    }

    // Position resolution
    if (hit.depth) {
        Vec3 movement = hit.normal * hit.depth;
        a->position += movement * a->mass / total_mass;
        b->position -= movement * b->mass / total_mass;
    }
}

void PhysicsEngine::add_box(AABody b) {
    bodies.push_back(b);
}

void PhysicsEngine::update(real delta) {
    real passed_t = 0;
    const int MAX_COLLISIONS = 100;

    // Find at max MAX_COLLISIONS and solve them in order.
    //
    // TODO(ed): If we reach the end of our checking with there
    // potentially being more collisions. We should solve _ALL_
    // collisions so we don't fall through the floor forexample.
    //
    // NOTE(ed): This is also a huge performance hog and it
    // will probably need to be fixed on a later date.
    for (int loop_breaker = 0; loop_breaker < MAX_COLLISIONS; loop_breaker++) {
        // Invalid collision that is past the current timestep.
        real t_left = 1.0 - passed_t;
        Manifold closest_hit = { .t = 2 };
        for (u32 outer = 0; outer < bodies.size(); outer++) {
            AABody *a = &bodies[outer];
            for (u32 inner = outer + 1; inner < bodies.size(); inner++) {
                AABody *b = &bodies[inner];
                if (a->mass == 0 && b->mass == 0) continue;
                Manifold hit_candidate = check_collision(a, b, delta);
                if (hit_candidate.depth) {
                    solve_collision(hit_candidate, delta);
                } else if (MARGIN < hit_candidate.t
                           && hit_candidate.t < t_left
                           && hit_candidate.t < closest_hit.t) {
                    closest_hit = hit_candidate;
                }
            }
        }

        if (closest_hit.t < 1.0) {
            for (AABody &a : bodies) {
                a.integrate_part(closest_hit.t, delta);
            }
            solve_collision(closest_hit, delta);
            passed_t += closest_hit.t;
        } else {
            break;
        }
    }

    // Move the bodies to the end of the step.
    for (AABody &a : bodies) {
        a.integrate(delta);
    }
}

void PhysicsEngine::draw() {
    for (AABody &a : bodies) {
        draw_aabody(a, Vec4(1.0, 0.0, 1.0, 1.0));
    }
}
}
