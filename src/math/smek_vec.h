#pragma once
#include "types.h"
#include "smek_math.h"
#include "../util/util.h"

///# Vectors
//

// Deliberately placed outside of namespace.
struct Vec2 {
    Vec2(real x = 0.0, real y = 0.0)
        : x(x)
        , y(y) {}

    union {
        real _[2];
        struct {
            real x, y;
        };
        struct {
            real r, g;
        };
    };
};
struct Vec3 {
    Vec3(real x = 0.0, real y = 0.0, real z = 0.0)
        : x(x)
        , y(y)
        , z(z) {}

    union {
        real _[3];
        struct {
            real x, y, z;
        };
        struct {
            real r, g, b;
        };
    };
};

struct Vec4 {
    Vec4(real x = 0.0, real y = 0.0, real z = 0.0, real w = 0.0)
        : x(x)
        , y(y)
        , z(z)
        , w(w) {}

    union {
        real _[4];
        struct {
            real x, y, z, w;
        };
        struct {
            real r, g, b, a;
        };
    };
};

///*
// Formats a vector to readable output.
i32 format(char *buffer, u32 size, FormatHint args, Vec2 v);
i32 format(char *buffer, u32 size, FormatHint args, Vec3 v);
i32 format(char *buffer, u32 size, FormatHint args, Vec4 v);

#if 0

///*
struct Vec2 {
    Vec2(real x=0.0, real y=0.0);
}


///*
struct Vec2 {
    Vec2(real x=0.0, real y=0.0, z=0.0);
}


///*
struct Vec4 {
    Vec4(real x=0.0, real y=0.0, z=0.0, w=0.0);
}

#endif

template <typename T>
constexpr int DIM();

template <typename T>
T operator+(const T &a, const T &b);

template <typename T>
T operator+=(T &a, const T &b);

template <typename T>
T operator*(const T &a, const real &s);

template <typename T>
T operator*(const real &s, const T &a);

template <typename T>
T operator/(const T &a, const real &s);

///*
template <typename T>
T operator-(const T &a);

template <typename T>
T operator-=(T &a, const T &b);

template <typename T>
T hadamard(const T &a, const T &b);

///*
template <typename T>
real dot(const T &a, const T &b);

///*
template <typename T>
real length(const T &a);

///*
template <typename T>
real length_squared(const T &a);

///*
template <typename T>
T normalized(const T &a);

///*
Vec3 cross(const Vec3 &a, const Vec3 &b);

template <typename T>
bool _close_enough_vec(const T &a, const T &b, real r);

///*
bool close_enough(const Vec2 &a, const Vec2 &b, real r = 0.001);
bool close_enough(const Vec3 &a, const Vec3 &b, real r = 0.001);
bool close_enough(const Vec4 &a, const Vec4 &b, real r = 0.001);

template <typename T>
constexpr int DIM() {
    T t;
    return sizeof(t._) / sizeof(t._[0]);
}

template <typename T>
T operator+(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] + b._[i];
    return result;
}

template <typename T>
T operator+=(T &a, const T &b) {
    for (i32 i = 0; i < DIM<T>(); i++) a._[i] += b._[i];
    return a;
}

template <typename T>
T operator-(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] - b._[i];
    return result;
}

template <typename T>
T operator-=(T &a, const T &b) {
    for (i32 i = 0; i < DIM<T>(); i++) a._[i] -= b._[i];
    return a;
}

template <typename T>
T operator*(const T &a, const real &s) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * s;
    return result;
}

template <typename T>
T operator*(const real &s, const T &a) {
    return a * s;
}

template <typename T>
T operator/(const T &a, const real &s) {
    return a * (1.0 / s);
}

template <typename T>
T operator-(const T &a) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = -a._[i];
    return result;
}

template <typename T>
T hadamard(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * b._[i];
    return result;
}

template <typename T>
real dot(const T &a, const T &b) {
    real result = 0;
    for (i32 i = 0; i < DIM<T>(); i++) result += a._[i] * b._[i];
    return result;
}

template <typename T>
real length(const T &a) {
    return Math::sqrt(length_squared(a));
}

template <typename T>
real length_squared(const T &a) {
    return dot(a, a);
}

template <typename T>
T normalized(const T &a) {
    return a / length(a);
}

template <typename T>
bool _close_enough_vec(const T &a, const T &b, real r) {
    bool result = true;
    for (i32 i = 0; i < DIM<T>(); i++)
        result &= Math::close_enough(a._[i], b._[i], r);
    return result;
}
