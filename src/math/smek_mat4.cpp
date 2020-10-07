#include "smek_mat4.h"
#include "../test.h"
#include "smek_math.h"
#include "../util/util.h"
#include "../renderer/renderer.h"

// Helper functions
static bool close_enough(const Mat &a, const Mat &b, real r = 0.001) {
    bool success = true;
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            success &= Math::close_enough(a._[i][j], b._[i][j], r);
        }
    }
    return success;
}

i32 format(char *buffer, u32 size, FormatHint args, Mat &m) {
    u32 p = args.num_decimals;
    return snprintf(buffer, size, "\n%.*f %.*f %.*f %.*f\n%.*f %.*f %.*f %.*f\n%.*f %.*f %.*f %.*f\n%.*f %.*f %.*f %.*f",
                    p, m._[0][0], p, m._[0][1], p, m._[0][2], p, m._[0][3],
                    p, m._[1][0], p, m._[1][1], p, m._[1][2], p, m._[1][3],
                    p, m._[2][0], p, m._[2][1], p, m._[2][2], p, m._[2][3],
                    p, m._[3][0], p, m._[3][1], p, m._[3][2], p, m._[3][3]);
}

void Mat::gfx_dump(Vec4 color) {
    Vec3 o = *this * Vec3(0, 0, 0);
    Vec3 x = *this * Vec3(1, 0, 0);
    Vec3 y = *this * Vec3(0, 1, 0);
    Vec3 z = *this * Vec3(0, 0, 1);
    GFX::push_line(o, x, color, Vec4(1, 0, 0, 1), 0.01);
    GFX::push_line(o, y, color, Vec4(0, 1, 0, 1), 0.01);
    GFX::push_line(o, z, color, Vec4(0, 0, 1, 1), 0.01);
}

Mat operator*(const Mat &a, const Mat &b) {
    Mat result = {};
    for (u32 row = 0; row < 4; row++) {
        for (u32 col = 0; col < 4; col++) {
            for (u32 k = 0; k < 4; k++) {
                result._[row][col] += a._[row][k] * b._[k][col];
            }
        }
    }
    return result;
}

Vec4 operator*(const Mat &m, const Vec4 &v) {
    Vec4 result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 k = 0; k < 4; k++) {
            result._[i] += v._[k] * m._[i][k];
        }
    }
    return result;
}

Vec3 operator*(const Mat &m, const Vec3 &v) {
    Vec4 r = m * Vec4(v.x, v.y, v.z, 1.0);
    return Vec3(r.x, r.y, r.z);
}

TEST_CASE("mat_mul", {
    Mat m = Mat::scale(2) * Mat::scale(5);
    bool success = true;
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 3; j++) {
            success &= m._[i][j] == 10 * (i == j);
        }
    }
    return success;
});

TEST_CASE("mat_mul", {
    Mat r = Mat::from(1, 2, 3, 4,
                      4, 3, 2, 1,
                      3, 4, 1, 2,
                      2, 1, 4, 3)
            * Mat::scale(2);
    return close_enough(r, Mat::from(2, 4, 6, 4,
                                     8, 6, 4, 1,
                                     6, 8, 2, 2,
                                     4, 2, 8, 3));
});

TEST_CASE("mat_mul", {
    Mat a = Mat::from(0, 1, 1, 0,
                      0, 0, 0, 0,
                      1, 0, 0, 0,
                      0, 1, 0, 1);

    Mat b = Mat::from(0, 0, 0, 1,
                      1, 0, 1, 1,
                      0, 1, 0, 1,
                      0, 0, 1, 0);

    Mat c = a * b;
    return close_enough(c, Mat::from(1, 1, 1, 2,
                                     0, 0, 0, 0,
                                     0, 0, 0, 1,
                                     1, 0, 2, 1));
});

Mat Mat::look_at(Vec3 from, Vec3 to, Vec3 up) {
    Vec3 z = normalized(from - to);
    Vec3 x = cross(normalized(up), z);
    Vec3 y = cross(z, x);
    Mat m = Mat::from(x.x, x.y, x.z, -dot(x, from),
                      y.x, y.y, y.z, -dot(y, from),
                      z.x, z.y, z.z, -dot(z, from),
                      0.0, 0.0, 0.0, 1.0);
    return m;
}

TEST_STMT("mat_look_at",
          close_enough(Mat::look_at(Vec3(0, 1, 0), Vec3(1, 1, 0), Vec3(0, 1, 0)),
                       Mat::from(+0, +0, +1, +0,
                                 +0, +1, +0, -1,
                                 -1, +0, +0, +0,
                                 +0, +0, +0, +1)));

TEST_STMT("mat_look_at",
          close_enough(Mat::look_at(Vec3(0, 0, 0), Vec3(0, 0, -1), Vec3(0, 1, 0)),
                       Mat::from(1, 0, 0, 0,
                                 0, 1, 0, 0,
                                 0, 0, 1, 0,
                                 0, 0, 0, 1)));

Mat Mat::perspective(real fov, real aspect_ratio, real near_clipping, real far_clipping) {
    real s = 1.0 / Math::tan(fov / 2);
    Mat result = {};
    result._[0][0] = s;
    result._[1][1] = s * aspect_ratio;
    result._[2][2] = -far_clipping / (far_clipping - near_clipping);
    result._[2][3] = -far_clipping * near_clipping / (far_clipping - near_clipping);
    result._[3][2] = -1;
    result._[3][3] = 0;
    return result;
}

Mat Mat::translate(real dx, real dy, real dz) {
    Mat result = scale(1);
    result._[0][3] = dx;
    result._[1][3] = dy;
    result._[2][3] = dz;
    return result;
}

Mat Mat::translate(Vec3 delta) {
    return translate(delta.x, delta.y, delta.z);
}

Mat Mat::rotate_x(real d) {
    real c = Math::cos(d);
    real s = Math::sin(d);
    return Mat::from(+1, +0, +0, +0,
                     +0, +c, -s, +0,
                     +0, +s, +c, +0,
                     +0, +0, +0, +1);
}

Mat Mat::rotate_y(real d) {
    real c = Math::cos(d);
    real s = Math::sin(d);
    return Mat::from(+c, +0, +s, +0,
                     +0, +1, +0, +0,
                     -s, +0, +c, +0,
                     +0, +0, +0, +1);
}

Mat Mat::rotate_z(real d) {
    real c = Math::cos(d);
    real s = Math::sin(d);
    return Mat::from(+c, -s, +0, +0,
                     +s, +c, +0, +0,
                     +0, +0, +1, +0,
                     +0, +0, +0, +1);
}

Mat Mat::rotate(real x, real y, real z) {
    return rotate_z(z) * rotate_y(y) * rotate_x(x);
}

Mat Mat::rotate(Vec3 axis) {
    return rotate(axis.x, axis.y, axis.z);
}

TEST_STMT("mat_translation",
          close_enough(Mat::translate(1, 2, 3), Mat::from(1, 0, 0, 1,
                                                          0, 1, 0, 2,
                                                          0, 0, 1, 3,
                                                          0, 0, 0, 1)));

Mat Mat::scale(real scale) {
    Mat result = {};
    for (u32 i = 0; i < 3; i++)
        result._[i][i] = scale;
    result._[3][3] = 1.0;
    return result;
}

Mat Mat::scale(Vec3 scale) {
    Mat result = {};
    for (u32 i = 0; i < 3; i++)
        result._[i][i] = scale._[i];
    result._[3][3] = 1.0;
    return result;
}

TEST_STMT("mat_scale",
          close_enough(Mat::scale(1), Mat::from(1, 0, 0, 0,
                                                0, 1, 0, 0,
                                                0, 0, 1, 0,
                                                0, 0, 0, 1)));

Mat Mat::transpose() {
    Mat result = {};
    for (i32 x = 0; x < 3; x++) {
        for (i32 y = 0; y < 3; y++) {
            result._[x][y] = _[y][x];
        }
    }
    return result;
}

// clang-format off
Mat Mat::invert() {
	Mat inv = {};

	inv.__[0] = __[5]  * __[10] * __[15] -
		__[5]  * __[11] * __[14] -
		__[9]  * __[6]  * __[15] +
		__[9]  * __[7]  * __[14] +
		__[13] * __[6]  * __[11] -
		__[13] * __[7]  * __[10];

    inv.__[4] = -__[4]  * __[10] * __[15] +
		__[4]  * __[11] * __[14] +
		__[8]  * __[6]  * __[15] -
		__[8]  * __[7]  * __[14] -
		__[12] * __[6]  * __[11] +
		__[12] * __[7]  * __[10];

    inv.__[8] = __[4]  * __[9] * __[15] -
		__[4]  * __[11] * __[13] -
		__[8]  * __[5] * __[15] +
		__[8]  * __[7] * __[13] +
		__[12] * __[5] * __[11] -
		__[12] * __[7] * __[9];

    inv.__[12] = -__[4]  * __[9] * __[14] +
		__[4]  * __[10] * __[13] +
		__[8]  * __[5] * __[14] -
		__[8]  * __[6] * __[13] -
		__[12] * __[5] * __[10] +
		__[12] * __[6] * __[9];

    inv.__[1] = -__[1]  * __[10] * __[15] +
		__[1]  * __[11] * __[14] +
		__[9]  * __[2] * __[15] -
		__[9]  * __[3] * __[14] -
		__[13] * __[2] * __[11] +
		__[13] * __[3] * __[10];

    inv.__[5] = __[0]  * __[10] * __[15] -
		__[0]  * __[11] * __[14] -
		__[8]  * __[2] * __[15] +
		__[8]  * __[3] * __[14] +
		__[12] * __[2] * __[11] -
		__[12] * __[3] * __[10];

    inv.__[9] = -__[0]  * __[9] * __[15] +
		__[0]  * __[11] * __[13] +
		__[8]  * __[1] * __[15] -
		__[8]  * __[3] * __[13] -
		__[12] * __[1] * __[11] +
		__[12] * __[3] * __[9];

    inv.__[13] = __[0]  * __[9] * __[14] -
		__[0]  * __[10] * __[13] -
		__[8]  * __[1] * __[14] +
		__[8]  * __[2] * __[13] +
		__[12] * __[1] * __[10] -
		__[12] * __[2] * __[9];

    inv.__[2] = __[1]  * __[6] * __[15] -
		__[1]  * __[7] * __[14] -
		__[5]  * __[2] * __[15] +
		__[5]  * __[3] * __[14] +
		__[13] * __[2] * __[7] -
		__[13] * __[3] * __[6];

    inv.__[6] = -__[0]  * __[6] * __[15] +
		__[0]  * __[7] * __[14] +
		__[4]  * __[2] * __[15] -
		__[4]  * __[3] * __[14] -
		__[12] * __[2] * __[7] +
		__[12] * __[3] * __[6];

    inv.__[10] = __[0]  * __[5] * __[15] -
		__[0]  * __[7] * __[13] -
		__[4]  * __[1] * __[15] +
		__[4]  * __[3] * __[13] +
		__[12] * __[1] * __[7] -
		__[12] * __[3] * __[5];

    inv.__[14] = -__[0]  * __[5] * __[14] +
		__[0]  * __[6] * __[13] +
		__[4]  * __[1] * __[14] -
		__[4]  * __[2] * __[13] -
		__[12] * __[1] * __[6] +
		__[12] * __[2] * __[5];

    inv.__[3] = -__[1] * __[6] * __[11] +
		__[1] * __[7] * __[10] +
		__[5] * __[2] * __[11] -
		__[5] * __[3] * __[10] -
		__[9] * __[2] * __[7] +
		__[9] * __[3] * __[6];

    inv.__[7] = __[0] * __[6] * __[11] -
		__[0] * __[7] * __[10] -
		__[4] * __[2] * __[11] +
		__[4] * __[3] * __[10] +
		__[8] * __[2] * __[7] -
		__[8] * __[3] * __[6];

    inv.__[11] = -__[0] * __[5] * __[11] +
		__[0] * __[7] * __[9] +
		__[4] * __[1] * __[11] -
		__[4] * __[3] * __[9] -
		__[8] * __[1] * __[7] +
		__[8] * __[3] * __[5];

    inv.__[15] = __[0] * __[5] * __[10] -
		__[0] * __[6] * __[9] -
		__[4] * __[1] * __[10] +
		__[4] * __[2] * __[9] +
		__[8] * __[1] * __[6] -
		__[8] * __[2] * __[5];

	f32 det = __[0] * inv.__[0] + __[1] * inv.__[4] + __[2] * inv.__[8] + __ [3] * inv.__[12];

	if (det == 0)
		return {};

	det = 1.0f / det;

	for (u8 i = 0; i < 16; i++)
		inv.__[i] *= det;

	return inv;
}
// clang-format off
