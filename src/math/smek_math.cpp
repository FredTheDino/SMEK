#include "smek_math.h"
#include "../test.h"

template<typename T>
T abs(T a) {
    return a > 0 ? a : -a;
}

TEST_STMT("abs_0", abs<i32>(-1) == 1);
TEST_STMT("abs_1", abs<f32>(1.0) == 1.0);
TEST_STMT("abs_2", abs<i32>(0) == 0);

template<typename T>
T min(T a, T b) {
    return a > b ? b : a;
}

TEST_STMT("min_0", min<f32>(-1.0, 1.0) == -1.0);
TEST_STMT("min_1", abs(min<f32>(0.01, 0.02) - 0.01) < 0.001);
TEST_STMT("min_2", min<i32>(-1.0, -2.0) == -2.0);

template<typename T>
T max(T a, T b) {
    return a < b ? b : a;
}

TEST_STMT("max_0", max<f32>(-1.0, 1.0) == 1.0);
TEST_STMT("max_1", abs(max<f32>(0.01, 0.02) - 0.02) < 0.001);
TEST_STMT("max_2", max<i32>(-1.0, -2.0) == -1.0);

template<typename T>
T sign(T a) {
    return 0 <= a ? 1 : -1;
}

TEST_STMT("sign_0", sign<f32>(-3.2) == -1.0);
TEST_STMT("sign_1", sign<f32>(0.02) == 1.0);
TEST_STMT("sign_2", sign<i32>(0.0) == 1.0);

template<typename T>
T sign_with_zero(T a) {
    if (a == 0) return 0;
    return sign(a);
}

TEST_STMT("sign_with_zero_0", sign_with_zero<f32>(-3.2) == -1.0);
TEST_STMT("sign_with_zero_1", sign_with_zero<f32>(0.02) == 1.0);
TEST_STMT("sign_with_zero_2", sign_with_zero<i32>(0) == 0);


