#include "smek_mat4.h"

Mat operator *(const Mat &a, const Mat &b) {
    Mat result = {};
    for (u32 i = 0; i < 4; i++) {
        for (u32 j = 0; j < 4; j++) {
            for (u32 k = 0; k < 4; k++) {
                result._[i][j] += a._[i][k] * a._[k][j];
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

