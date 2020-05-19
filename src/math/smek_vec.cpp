#include "smek_vec.h"
#include "../test.h"

Vec3 cross(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.y * b.z - b.y * a.z,
                a.z * b.x - b.z * a.x,
                a.x * b.y - b.x * a.y);
}


bool close_enough(const Vec2 &a, const Vec2 &b, real r) {
    return _close_enough_vec(a, b, r);
}

bool close_enough(const Vec3 &a, const Vec3 &b, real r) {
    return _close_enough_vec(a, b, r);
}

bool close_enough(const Vec4 &a, const Vec4 &b, real r) {
    return _close_enough_vec(a, b, r);
}

TEST_STMT("vec_add", close_enough(Vec3(3, 4, 2) + Vec3(1, -2, 2), Vec3(4, 2, 4)));
TEST_STMT("vec_add", close_enough(Vec4() + Vec4(1, 1, 1), Vec4(1, 1, 1)));
TEST_STMT("vec_comp", DIM<Vec2>() == 2);
TEST_STMT("vec_comp", DIM<Vec3>() == 3);
TEST_STMT("vec_comp", DIM<Vec4>() == 4);
TEST_STMT("vec_cross", close_enough(cross(Vec3(1, 0, 0), Vec3(0, 1, 0)), Vec3(0, 0, 1)));
TEST_STMT("vec_div", close_enough(Vec4(1, 3, 3, 7) / 1, Vec4(1, 3, 3, 7)));
TEST_STMT("vec_div", close_enough(Vec4(3, 3, 3, 9) / 3, Vec4(1, 1, 1, 3)));
TEST_STMT("vec_dot", Math::close_enough<real>(dot(Vec2(0, 1), Vec2(1, 0)), 0.0, 0.001));
TEST_STMT("vec_dot", Math::close_enough<real>(dot(Vec4(0, 1), Vec4(1, 0)), 0.0, 0.001));
TEST_STMT("vec_dot", close_enough(cross(Vec3(1, 0, 0), Vec3(0, 1, 0)), Vec3(0, 0, 1)));
TEST_STMT("vec_hadamard", close_enough(hadamard(Vec2(1, 2), Vec2(3, 4)), Vec2(3, 8)));
TEST_STMT("vec_length", Math::close_enough<real>(length(Vec4(3, 4)), 5.0, 0.001));
TEST_STMT("vec_length_squared", Math::close_enough<real>(length_squared(Vec2(1, 1)), 2.0, 0.001));
TEST_STMT("vec_length_squared", Math::close_enough<real>(length_squared(Vec4(1, 2, 3, 4)), 30.0, 0.001));
TEST_STMT("vec_mul", close_enough(0 * Vec4(1, 1, 3, 7), Vec4()));
TEST_STMT("vec_mul", close_enough(Vec2(1, 1) * 3, Vec2(3, 3)));
TEST_STMT("vec_normalized", close_enough(normalized(Vec4(1, 1, 1, 1)), Vec4(0.5, 0.5, 0.5, 0.5), 0.001));
TEST_STMT("vec_sub", close_enough(Vec2(3, 4) - Vec2(1, -2), Vec2(2, 6)));
TEST_STMT("vec_sub", close_enough(Vec4() - Vec4(1, 1, 1), Vec4(-1, -1, -1)));
