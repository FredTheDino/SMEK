#include "smek_vec.h"
#include "../test.h"

template<typename T>
constexpr int DIM() {
    T t; return sizeof(t._) / sizeof(t._[0]);
}

TEST_STMT("vec_comp", DIM<Vec2>() == 2);
TEST_STMT("vec_comp", DIM<Vec3>() == 3);
TEST_STMT("vec_comp", DIM<Vec4>() == 4);

template<typename T>
T operator +(const T &a, const T &b) {
    T result;
    for (u32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] + b._[i];
    return result;
}

TEST_STMT("vec_add", close_enough(Vec3(3, 4, 2) + Vec3(1, -2, 2), Vec3(4, 2, 4)));
TEST_STMT("vec_add", close_enough(Vec4() + Vec4(1, 1, 1), Vec4(1, 1, 1)));

template<typename T>
T operator -(const T &a, const T &b) {
    T result;
    for (u32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] - b._[i];
    return result;
}

TEST_STMT("vec_sub", close_enough(Vec2(3, 4) - Vec2(1, -2), Vec2(2, 6)));
TEST_STMT("vec_sub", close_enough(Vec4() - Vec4(1, 1, 1), Vec4(-1, -1, -1)));


template<typename T>
T operator *(const T &a, const real &s) {
    T result;
    for (u32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * s;
    return result;
}

TEST_STMT("vec_mul", close_enough(Vec2(1, 1) * 3, Vec2(3, 3)));

template<typename T>
T operator *(const real &s, const T &a) {
    return a * s;
}

TEST_STMT("vec_mul", close_enough(0 * Vec4(1, 1, 3, 7), Vec4()));

template<typename T>
T operator /(const T &a, const real &s) {
    return a * (1.0 / s);
}

TEST_STMT("vec_div", close_enough(Vec4(1, 3, 3, 7) / 1, Vec4(1, 3, 3, 7)));
TEST_STMT("vec_div", close_enough(Vec4(3, 3, 3, 9) / 3, Vec4(1, 1, 1, 3)));

template<typename T>
T hadamard(const T &a, const T &b) {
    T result;
    for (u32 i = 0; i < DIM<T>(); i++) result._[i] = a._[i] * b._[i];
    return result;
}

TEST_STMT("vec_hadamard", close_enough(hadamard(Vec2(1, 2), Vec2(3, 4)), Vec2(3, 8)));

template<typename T>
real dot(const T &a, const T &b) {
    real result = 0;
    for (u32 i = 0; i < DIM<T>(); i++) result += a._[i] * b._[i];
    return result;
}

using Math::close_enough;
TEST_STMT("vec_dot", close_enough<real>(dot(Vec2(0, 1), Vec2(1, 0)), 0.0, 0.001));
TEST_STMT("vec_dot", close_enough<real>(dot(Vec4(0, 1), Vec4(1, 0)), 0.0, 0.001));

template<typename T>
bool _close_enough_vec(const T &a, const T &b, real r) {
    bool result = true;
    for (u32 i = 0; i < DIM<T>(); i++)
        result &= Math::close_enough(a._[i], b._[i], r);
    return result;
}
