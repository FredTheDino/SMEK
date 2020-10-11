#include "smek_vec.h"
#include "../test.h"

void Vec2::to(real *arr) const {
    arr[0] = x;
    arr[1] = y;
}

void Vec3::to(real *arr) const {
    arr[0] = x;
    arr[1] = y;
    arr[2] = z;
}

void Vec4::to(real *arr) const {
    arr[0] = x;
    arr[1] = y;
    arr[2] = z;
    arr[3] = w;
}

void Color3::to(real *arr) const {
    arr[0] = x;
    arr[1] = y;
    arr[2] = z;
}

void Color4::to(real *arr) const {
    arr[0] = x;
    arr[1] = y;
    arr[2] = z;
    arr[3] = w;
}

real &Vec2::operator[](std::size_t idx) {
    ASSERT_LT(idx, 2);
    return _[idx];
}
real &Vec3::operator[](std::size_t idx) {
    ASSERT_LT(idx, 3);
    return _[idx];
}
real &Vec4::operator[](std::size_t idx) {
    ASSERT_LT(idx, 4);
    return _[idx];
}

Vec3 cross(const Vec3 &a, const Vec3 &b) {
    return Vec3(a.y * b.z - b.y * a.z,
                a.z * b.x - b.z * a.x,
                a.x * b.y - b.x * a.y);
}

i32 format(char *buffer, u32 size, FormatHint args, Vec2 v) {
    return snprintf(buffer, size, "(%0*.*f, %0*.*f)",
                    args.num_zero_pad, args.num_decimals, v.x,
                    args.num_zero_pad, args.num_decimals, v.y);
}

i32 format(char *buffer, u32 size, FormatHint args, Vec3 v) {
    return snprintf(buffer, size, "(%0*.*f, %0*.*f, %0*.*f)",
                    args.num_zero_pad, args.num_decimals, v.x,
                    args.num_zero_pad, args.num_decimals, v.y,
                    args.num_zero_pad, args.num_decimals, v.z);
}

i32 format(char *buffer, u32 size, FormatHint args, Vec4 v) {
    return snprintf(buffer, size, "(%0*.*f, %0*.*f, %0*.*f, %0*.*f)",
                    args.num_zero_pad, args.num_decimals, v.x,
                    args.num_zero_pad, args.num_decimals, v.y,
                    args.num_zero_pad, args.num_decimals, v.z,
                    args.num_zero_pad, args.num_decimals, v.w);
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

TEST_CASE("vec2[]_read", {
    Vec2 v(1, 2);
    return v[0] == 1 && v[1] == 2;
});
TEST_CASE("vec2[]_assign", {
    Vec2 v(0, 0);
    v[0] = 1;
    v[1] = 2;
    return close_enough(v, Vec2(1, 2));
});
TEST_CASE("vec3[]_read", {
    Vec3 v(1, 2, 3);
    return v[0] == 1 && v[1] == 2 && v[2] == 3;
});
TEST_CASE("vec3[]_assign", {
    Vec3 v(0, 0, 0);
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    return close_enough(v, Vec3(1, 2, 3));
});
TEST_CASE("vec4[]_read", {
    Vec4 v(1, 2, 3, 4);
    return v[0] == 1 && v[1] == 2 && v[2] == 3 && v[3] == 4;
});
TEST_CASE("vec4[]_assign", {
    Vec4 v(0, 0, 0, 0);
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    v[3] = 4;
    return close_enough(v, Vec4(1, 2, 3, 4));
});

TEST_FORMAT(Vec2(1, 1.5), "(1.00, 1.50)", .num_decimals = 2);
TEST_FORMAT(Vec2(1.04, -1.5), "(1.0, -1.5)", .num_decimals = 1);
TEST_FORMAT(Vec3(1.000, 1.4, 2.511), "(1.00, 1.40, 2.51)", .num_decimals = 2);
TEST_FORMAT(Vec3(-1.0, -2.56, -10.0005), "(-1.0, -2.6, -10.0)", .num_decimals = 1);
TEST_FORMAT(Vec4(-1, -2, 3, 2), "(-1, -2, 3, 2)", .num_decimals = 0);
TEST_FORMAT(Vec4(-1.1, -2.1, 3.8, 2.8), "(-1, -2, 4, 3)", .num_decimals = 0);

TEST_CASE("Vec2[] read", {
    Vec2 v(1, 2);
    return v[0] == 1 && v[1] == 2;
});
TEST_CASE("Vec2[] assign", {
    Vec2 v(0, 0);
    v[0] = 1;
    v[1] = 2;
    return close_enough(v, Vec2(1, 2));
});
TEST_CASE("Vec3[] read", {
    Vec3 v(1, 2, 3);
    return v[0] == 1 && v[1] == 2 && v[2] == 3;
});
TEST_CASE("Vec3[] assign", {
    Vec3 v(0, 0, 0);
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    return close_enough(v, Vec3(1, 2, 3));
});
TEST_CASE("Vec4[] read", {
    Vec4 v(1, 2, 3, 4);
    return v[0] == 1 && v[1] == 2 && v[2] == 3 && v[3] == 4;
});
TEST_CASE("Vec4[] assign", {
    Vec4 v(0, 0, 0, 0);
    v[0] = 1;
    v[1] = 2;
    v[2] = 3;
    v[3] = 4;
    return close_enough(v, Vec4(1, 2, 3, 4));
});

TEST_CASE("Vec2 to", {
    Vec2 v(1, 2);
    real arr[2];
    v.to(arr);
    return arr[0] == 1 && arr[1] == 2;
});
TEST_CASE("Vec3 to", {
    Vec3 v(1, 2, 3);
    real arr[3];
    v.to(arr);
    return arr[0] == 1 && arr[1] == 2 && arr[2] == 3;
});
TEST_CASE("Vec4 to", {
    Vec4 v(1, 2, 3, 4);
    real arr[4];
    v.to(arr);
    return arr[0] == 1 && arr[1] == 2 && arr[2] == 3 && arr[3] == 4;
});

TEST_CASE("Vec2 from array", {
    Vec2 v({ 1, 2 });
    return v[0] == 1 && v[1] == 2;
});
TEST_CASE("Vec3 from array", {
    Vec3 v({ 1, 2, 3 });
    return v[0] == 1 && v[1] == 2 && v[2] == 3;
});
TEST_CASE("Vec2 from array", {
    Vec4 v({ 1, 2, 3, 4});
    return v[0] == 1 && v[1] == 2 && v[2] == 3 && v[3] == 4;
});
