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
    Vec2(real *arr)
        : x(arr[0])
        , y(arr[1]) {}

    union {
        real _[2];
        struct {
            real x, y;
        };
    };

    void to(real *arr) const;
    real &operator[](std::size_t idx);
};

struct Vec3 {
    Vec3(real x = 0.0, real y = 0.0, real z = 0.0)
        : x(x)
        , y(y)
        , z(z) {}
    Vec3(real *arr)
        : x(arr[0])
        , y(arr[1])
        , z(arr[2]) {}

    union {
        real _[3];
        struct {
            real x, y, z;
        };
    };

    void to(real *arr) const;
    real &operator[](std::size_t idx);

    static Vec3 from(real *arr) { return { arr[0], arr[1], arr[2] }; }
};

struct Vec4 {
    Vec4(real x = 0.0, real y = 0.0, real z = 0.0, real w = 0.0)
        : x(x)
        , y(y)
        , z(z)
        , w(w) {}
    Vec4(real *arr)
        : x(arr[0])
        , y(arr[1])
        , z(arr[2])
        , w(arr[3]) {}

    union {
        real _[4];
        struct {
            real x, y, z, w;
        };
    };

    void to(real *arr) const;
    real &operator[](std::size_t idx);

    static Vec4 from(real *arr) { return { arr[0], arr[1], arr[2], arr[3] }; }
};

struct Color4;
struct Color3 {
    Color3(real r = 0.0, real g = 0.0, real b = 0.0)
        : r(r)
        , g(g)
        , b(b) {}
    Color3(Vec3 source)
        : r(source.x)
        , g(source.y)
        , b(source.z) {}
    Color3(real *arr)
        : r(arr[0])
        , g(arr[1])
        , b(arr[2]) {}

    union {
        real _[3];
        struct {
            real r, g, b;
        };
    };

    operator Color4() const;

    void to(real *arr) const;
    real &operator[](std::size_t idx);

    static Color3 from(real *arr) { return { arr[0], arr[1], arr[2] }; };
};

struct Color4 {
    Color4(real r = 0.0, real g = 0.0, real b = 0.0, real a = 0.0)
        : r(r)
        , g(g)
        , b(b)
        , a(a) {}
    Color4(Vec4 source)
        : r(source.x)
        , g(source.y)
        , b(source.z)
        , a(source.w) {}
    Color4(real *arr)
        : r(arr[0])
        , g(arr[1])
        , b(arr[2])
        , a(arr[3]) {}
    union {
        real _[4];
        struct {
            real r, g, b, a;
        };
    };

    // Explicit since the alpha channel is destroyed.
    explicit operator Color3() const {
        return Color3(r, g, b);
    }

    void to(real *arr) const;
    real &operator[](std::size_t idx);

    static Color4 from(real *arr) { return { arr[0], arr[1], arr[2], arr[3] }; }
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
struct Vec3 {
    Vec3(real x=0.0, real y=0.0, real z=0.0);
}


///*
struct Vec4 {
    Vec4(real x=0.0, real y=0.0, real z=0.0, real w=0.0);
}

#endif

// clang-format off
template <typename T>
concept VectorType = std::same_as<T, Vec2>
                  || std::same_as<T, Vec3>
                  || std::same_as<T, Vec4>
                  || std::same_as<T, Color3>
                  || std::same_as<T, Color4>;

template <typename T>
concept ColorType = std::same_as<T, Color3>
                 || std::same_as<T, Color4>;
// clang-format on

template <VectorType T>
constexpr int DIM();

template <VectorType T>
T operator+(const T &a, const T &b);

template <VectorType T>
T operator+=(T &a, const T &b);

template <VectorType T>
T operator*=(T &a, const real &b);

template <VectorType T>
T operator/=(T &a, const real &b);

template <VectorType T>
T operator*(const T &a, const real &s);

template <VectorType T>
T operator*(const real &s, const T &a);

template <VectorType T>
T operator/(const T &a, const real &s);

///*
template <VectorType T>
T operator-(const T &a);

template <VectorType T>
T operator-=(T &a, const T &b);

template <VectorType T>
T hadamard(const T &a, const T &b);

///*
template <VectorType T>
real dot(const T &a, const T &b);

///*
template <VectorType T>
real length(const T &a);

///*
template <VectorType T>
real length_squared(const T &a);

///*
template <VectorType T>
T normalized(const T &a);

///*
template <typename T>
T abs(const T &a);

///*
Vec3 cross(const Vec3 &a, const Vec3 &b);

template <VectorType T>
bool _close_enough_vec(const T &a, const T &b, real r);

///*
bool close_enough(const Vec2 &a, const Vec2 &b, real r = 0.001);
bool close_enough(const Vec3 &a, const Vec3 &b, real r = 0.001);
bool close_enough(const Vec4 &a, const Vec4 &b, real r = 0.001);

template <VectorType T>
constexpr int DIM() {
    T t;
    return sizeof(t._) / sizeof(t._[0]);
}

template <VectorType T>
T operator+(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] + b._[i];
    return result;
}

template <VectorType T>
T operator+=(T &a, const T &b) {
    for (i32 i = 0; i < DIM<T>(); i++) a._[i] += b._[i];
    return a;
}

template <VectorType T>
T operator-(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] - b._[i];
    return result;
}

template <VectorType T>
T operator-=(T &a, const T &b) {
    for (i32 i = 0; i < DIM<T>(); i++) a._[i] -= b._[i];
    return a;
}

template <VectorType T>
T operator*=(T &a, const real &b) {
    a = a * b;
    return a;
}

template <VectorType T>
T operator/=(T &a, const real &b) {
    a = a / b;
    return a;
}

template <VectorType T>
T operator*(const T &a, const real &s) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * s;
    return result;
}

template <VectorType T>
T operator*(const real &s, const T &a) {
    return a * s;
}

template <VectorType T>
T operator/(const T &a, const real &s) {
    return a * (1.0 / s);
}

template <VectorType T>
T operator-(const T &a) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = -a._[i];
    return result;
}

template <VectorType T>
T hadamard(const T &a, const T &b) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * b._[i];
    return result;
}

template <VectorType T>
real dot(const T &a, const T &b) {
    real result = 0;
    for (i32 i = 0; i < DIM<T>(); i++) result += a._[i] * b._[i];
    return result;
}

template <VectorType T>
real length(const T &a) {
    return Math::sqrt(length_squared(a));
}

template <VectorType T>
real length_squared(const T &a) {
    return dot(a, a);
}

template <VectorType T>
T normalized(const T &a) {
    return a / length(a);
}

template <VectorType T>
bool _close_enough_vec(const T &a, const T &b, real r) {
    bool result = true;
    for (i32 i = 0; i < DIM<T>(); i++)
        result &= Math::close_enough(a._[i], b._[i], r);
    return result;
}

template <VectorType T>
T abs(const T &a) {
    T result;
    for (i32 i = 0; i < DIM<T>(); i++)
        result._[i] = Math::abs(a._[i]);
    return result;
};
