#include "smek_mat4.h"
#include "../test.h"
#include "smek_math.h"

// Helper functions
static bool close_enough(const Mat &a, const Mat &b, real r=0.001) {
    bool success = true;
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            success &= Math::close_enough(a._[i][j], b._[i][j], r);
        }
    }
    return success;
}

#include "../util/log.h"
static void log_matrix(const Mat &m) {
    LOG("\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f\n%.2f %.2f %.2f %.2f",
        m._[0][0], m._[0][1], m._[0][2], m._[0][3],
        m._[1][0], m._[1][1], m._[1][2], m._[1][3],
        m._[2][0], m._[2][1], m._[2][2], m._[2][3],
        m._[3][0], m._[3][1], m._[3][2], m._[3][3]);
}

Mat operator *(const Mat &a, const Mat &b) {
    Mat result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            for (u32 k = 0; k < 4; k++) {
                result._[i][j] += a._[i][k] * b._[k][j];
            }
        }
    }
    return result;
}

Vec4 operator *(const Vec4 &v, const Mat &m) {
    Vec4 result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 k = 0; k < 4; k++) {
            result._[i] += v._[k] * m._[i][k];
        }
    }
    return result;
}

Vec3 operator *(const Vec3 &v, const Mat &m) {
    Vec4 r = Vec4(v.x, v.y, v.z, 1.0) * m;
    return Vec3(r.x, r.y, r.z);
}

TEST_CASE("mat_mul", {
    Mat m = Mat::scale(2) * Mat::scale(5);
    bool success = true;
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            success &= m._[i][j] == 10 * (i == j);
        }
    }
    return success;
});

TEST_CASE("mat_mul", {
    Mat r = Mat::from(1, 2, 3, 4,
                      4, 3, 2, 1,
                      3, 4, 1, 2,
                      2, 1, 4, 3) * Mat::scale(2);
    return close_enough(r, Mat::from(2, 4, 6, 8,
                                     8, 6, 4, 2,
                                     6, 8, 2, 4,
                                     4, 2, 8, 6));
});

Mat Mat::look_towards(Vec3 from, Vec3 to, Vec3 up) {
    Vec3 w = normalized(from - to);
    Vec3 u = cross(w, normalized(up));
    Vec3 v = cross(w, u);
    Mat m = Mat::from(u.x, v.x, w.x, from.x,
                      u.y, v.y, w.y, from.y,
                      u.z, v.z, w.z, from.z,
                      0.0, 0.0, 0.0, 1.0);
    return m;
}

Mat Mat::perspective(real fov, real near, real far) {
    real s = 1.0 / Math::tan(fov / 2);
    Mat result = {};
    result._[0][0] = s;
    result._[1][1] = s;
    result._[2][2] = -far / (far - near);
    result._[2][3] = -far * near / (far - near);
    result._[3][2] = -1;
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

TEST_STMT("mat_translation",
    close_enough(Mat::translate(1, 2, 3), Mat::from(1, 0, 0, 1,
                                                    0, 1, 0, 2,
                                                    0, 0, 1, 3,
                                                    0, 0, 0, 1))
);

Mat Mat::scale(real scale) {
    Mat result = {};
    for (u32 i = 0; i < 4; i++)
        result._[i][i] = scale;
    return result;
}

TEST_STMT("mat_scale",
    close_enough(Mat::scale(1), Mat::from(1, 0, 0, 0,
                                          0, 1, 0, 0,
                                          0, 0, 1, 0,
                                          0, 0, 0, 1))
);

