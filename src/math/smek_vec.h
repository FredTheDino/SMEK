#pragma once
#include "types.h"
#include "smek_math.h"

// Deliberately placed outside of namespace.
struct Vec2 {
    Vec2(real x=0.0, real y=0.0):
        x(x), y(y) {}

    union {
        real _[2];
        struct { real x, y; };
        struct { real r, g; };
    };
};


struct Vec3 {
    Vec3(real x=0.0, real y=0.0, real z=0.0):
        x(x), y(y), z(z) {}

    union {
        real _[3];
        struct { real x, y, z; };
        struct { real r, g, b; };
    };
};

struct Vec4 {
    Vec4(real x=0.0, real y=0.0, real z=0.0, real w=0.0):
        x(x), y(y), z(z), w(w) {}

    union {
        real _[4];
        struct { real x, y, z, w; };
        struct { real r, g, b, a; };
    };
};

template<typename T>
constexpr int DIM();

template<typename T>
T operator +(const T &a, const T &b);

template<typename T>
T operator -(const T &a, const T &b);

template<typename T>
T operator *(const T &a, const real &s);

template<typename T>
T operator *(const real &s, const T &a);

template<typename T>
T operator /(const T &a, const real &s);

template<typename T>
T hadamard(const T &a, const T &b);

template<typename T>
real dot(const T &a, const T &b);

template<typename T>
bool _close_enough_vec(const T &a, const T &b, real r);

bool close_enough(const Vec2 &a, const Vec2 &b, real r=0.001);
bool close_enough(const Vec3 &a, const Vec3 &b, real r=0.001);
bool close_enough(const Vec4 &a, const Vec4 &b, real r=0.001);
