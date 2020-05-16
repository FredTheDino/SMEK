#include <cmath>
#include "smek_math.h"
#include "../test.h"

namespace Math {

template<typename T>
T abs(T a) {
    return a > 0 ? a : -a;
}

TEST_STMT("abs", abs<i32>(-1) == 1);
TEST_STMT("abs", abs<f32>(1.0) == 1.0);
TEST_STMT("abs", abs<i32>(0) == 0);

template<typename T>
T min(T a, T b) {
    return a > b ? b : a;
}

TEST_STMT("min", min<f32>(-1.0, 1.0) == -1.0);
TEST_STMT("min", abs(min<f32>(0.01, 0.02) - 0.01) < 0.001);
TEST_STMT("min", min<i32>(-1.0, -2.0) == -2.0);

template<typename T>
T max(T a, T b) {
    return a < b ? b : a;
}

TEST_STMT("max", max<f32>(-1.0, 1.0) == 1.0);
TEST_STMT("max", abs(max<f32>(0.01, 0.02) - 0.02) < 0.001);
TEST_STMT("max", max<i32>(-1.0, -2.0) == -1.0);

template<typename T>
T sign(T a) {
    return 0 <= a ? 1 : -1;
}

TEST_STMT("sign", sign<f32>(-3.2) == -1.0);
TEST_STMT("sign", sign<f32>(0.02) == 1.0);
TEST_STMT("sign", sign<i32>(0.0) == 1.0);

template<typename T>
T sign_with_zero(T a) {
    if (a == 0) return 0;
    return sign(a);
}

TEST_STMT("sign_with_zero", sign_with_zero<f32>(-3.2) == -1.0);
TEST_STMT("sign_with_zero", sign_with_zero<f32>(0.02) == 1.0);
TEST_STMT("sign_with_zero", sign_with_zero<i32>(0) == 0);

template<typename T>
bool close_enough(T a, T b, f32 r) {
    return abs(a - b) < r;
}

TEST_STMT("close_enough", close_enough(1, 1));
TEST_STMT("close_enough", close_enough(1, 2, 3));
TEST_STMT("close_enough", close_enough(0.0, 0.00005));

// These are just annoying to write yourself... So I don't bother.
real pow(real a, real b) { return std::pow(a, b); }

real sqrt(real a) { return std::sqrt(a); }

real mod(real a, real b) { return std::fmod(a, b); }

real rem(real a, real b) { return std::remainder(a, b); }

real exp(real a) { return std::exp(a); }

real log(real a) { return std::log(a); }

real sin(real a) { return std::sin(a); }

real cos(real a) { return std::cos(a); }

real tan(real a) { return std::tan(a); }

real asin(real a) { return std::asin(a); }

real acos(real a) { return std::acos(a); }

real atan(real a) { return std::atan(a); }

real atan2(real x, real y) { return std::atan2(y, x); }

template<typename R>
R ceil(real a) {
    if (a < 0)
        return R(a);
    else
        return R(a) + 1;
}

TEST_STMT("ceil", ceil<i32>(0.01f) == 1);
TEST_STMT("ceil", ceil<i32>(-0.01f) == 0);
TEST_STMT("ceil", ceil<i32>(20.445f) == 21);

template<typename R>
R floor(real a) {
    if (a < 0)
        return R(a) - 1;
    else
        return R(a);
}

TEST_STMT("floor", floor<i32>(0.01f) == 0);
TEST_STMT("floor", floor<i32>(-0.01f) == -1);
TEST_STMT("floor", floor<i32>(20.445f) == 20);

template<typename R>
R round(real a) {
    return floor<R>(a + 0.5);
}

TEST_STMT("round", round<i32>(0.01f) == 0);
TEST_STMT("round", round<i32>(0.5f) == 1);
TEST_STMT("round", round<i32>(-0.5f) == 0);
TEST_STMT("round", round<i32>(12.231f) == 12);

real to_radians(real degrees) {
    return degrees * PI / 180;
}

TEST_STMT("to_radians", close_enough(to_radians(0), 0.0f));
TEST_STMT("to_radians", close_enough(to_radians(1), 0.01745329f));
TEST_STMT("to_radians", close_enough(to_radians(-1), -0.01745329f));
TEST_STMT("to_radians", close_enough(to_radians(180), PI));

real to_degrees(real radians) {
    return radians * 180 / PI;
}

TEST_STMT("to_radians", close_enough(to_degrees(0), 0.0f));
TEST_STMT("to_radians", close_enough(to_degrees(0.01745329), 1.0f));
TEST_STMT("to_radians", close_enough(to_degrees(-0.01745329), -1.0f));
TEST_STMT("to_radians", close_enough(to_degrees(PI), 180.0f));
TEST_STMT("to_radians", close_enough(to_radians(to_degrees(123)), 123.0f));

} // namespace Math
