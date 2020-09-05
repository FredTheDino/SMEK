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

    // Ray origin inside bounding box
    //
    // NOTE(ed): Not returning a normal here
    // makes things more numerically unstable.
    // I didn't mange to think of a way to do this,
    // but there might be a smart way to do it here.
    // Otherwise we fall back on the "collision_check"
    // function for the "t" we know they are overlapping.
    if (inside)
        return { 0, origin };

    // Find Max T
    int plane = 0;
    if (max_t._[1] > max_t._[plane]) plane = 1;
    if (max_t._[2] > max_t._[plane]) plane = 2;

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

RayHit check_collision(Box *a, Box *b, real delta) {
    // Check
    Box extended_box = b->extend(a->half_size);

    Vec3 total_movement = (a->velocity - b->velocity) * delta;
    RayHit hit = collision_line_box(a->position, total_movement, extended_box);
    hit.a = a;
    hit.b = b;
    return hit;
}

const real MARGIN = 0.001;
void solve_collision(RayHit hit, real delta) {
    Box *a, *b;
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
}

bool check_and_solve_collision(Box *a, Box *b, real delta) {
    RayHit hit = check_collision(a, b, delta);

    if (!hit || hit.t < MARGIN)
        return false;

    draw_ray_hit(hit, Vec4(0.5, 0.2, 0.3, 1.0));

    solve_collision(hit, delta);
    return true;
}

void PhysicsEngine::add_box(Box b) {
    boxes.push_back(b);
}

void PhysicsEngine::step(real delta) {
    real passed_t = 0;
    int collisions = 0;
    for (int UPPER = 0; UPPER < 100 && passed_t < 1; UPPER++) {
        // Invalid collision that is past the current timestep
        RayHit closest_hit = { 2 };
        for (int outer = 0; outer < boxes.size(); outer++) {
            Box *a = &boxes[outer];
            for (int inner = outer + 1; inner < boxes.size(); inner++) {
                Box *b = &boxes[inner];
                if (a->mass == 0 && b->mass == 0) continue;
                RayHit hit_candidate = check_collision(a, b, delta);
                const bool is_collision = MARGIN < hit_candidate.t && hit_candidate.t < 1.0 - passed_t;
                const bool is_closest = hit_candidate.t < closest_hit.t;
                if (is_collision && is_closest) {
                    closest_hit = hit_candidate;
                }
            }
        }

        if (closest_hit) {
            LOG("HIT {} {}", collisions++, closest_hit.t);
            for (Box &a : boxes) {
                a.integrate_part(closest_hit.t, delta);
            }
            solve_collision(closest_hit, delta);
            passed_t += closest_hit.t;
        } else {
            break;
        }
    }

    for (Box &a : boxes) {
        a.integrate(delta);
    }
}

void PhysicsEngine::draw() {
    for (Box &a : boxes) {
        draw_box(a, Vec4(1.0, 0.0, 1.0, 1.0));
    }
}
