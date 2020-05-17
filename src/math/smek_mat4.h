#pragma once
#include "types.h"
#include "smek_vec.h"

// We only have 4x4 matricies, since that's kinda what we're using here...
struct Mat {
    real _[4][4];
};

Mat operator *(const Mat &a, const Mat &b);

Vec4 operator *(const Vec4 &v, const Mat &m);

Vec3 operator *(const Vec3 &v, const Mat &m);

// TODO(ed): Perspective, translation, rotation, scale.
