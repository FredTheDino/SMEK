#pragma once
#include "types.h"
#include "smek_vec.h"
#include "smek_quat.h"
#include "../util/util.h"

///# 4x4 matrix
//

// We only have 4x4 matricies, since that's kinda what we're using here...
struct Mat {
    union {
        real _[4][4];
        real __[16];
    };

    static Mat from(real a0, real a1, real a2, real a3,
                    real a4, real a5, real a6, real a7,
                    real a8, real a9, real a10, real a11,
                    real a12, real a13, real a14, real a15) {
        Mat m;
        m._[0][0] = a0;
        m._[0][1] = a1;
        m._[0][2] = a2;
        m._[0][3] = a3;
        m._[1][0] = a4;
        m._[1][1] = a5;
        m._[1][2] = a6;
        m._[1][3] = a7;
        m._[2][0] = a8;
        m._[2][1] = a9;
        m._[2][2] = a10;
        m._[2][3] = a11;
        m._[3][0] = a12;
        m._[3][1] = a13;
        m._[3][2] = a14;
        m._[3][3] = a15;
        return m;
    }

    real *data() { return &_[0][0]; }

    static Mat scale(real scale);
    static Mat translate(real dx, real dy, real dz);
    static Mat translate(Vec3 delta);
    static Mat rotate_x(real d);
    static Mat rotate_y(real d);
    static Mat rotate_z(real d);
    static Mat rotate(real dx, real dy, real dz);
    static Mat rotate(Vec3 delta);
    static Mat look_at(Vec3 from, Vec3 to, Vec3 up);
    static Mat perspective(real fov, real aspect_ratio, real near, real far);

    static Mat from(H h);

    Mat invert();
};

///*
// Formats a matrix to readable output.
int format(char *buffer, FormatHint args, Mat &m);

#if 0

///*
Mat Mat::scale(real scale);

///*
Mat Mat::translate(real dx, real dy, real dz);

///*
Mat Mat::translate(Vec3 delta);

///*
Mat Mat::look_at(Vec3 from, Vec3 to, Vec3 up);

///*
Mat Mat::perspective(real fov, real near, real far);

#endif

Mat operator *(const Mat &a, const Mat &b);

Vec4 operator *(const Mat &m, const Vec4 &v);

Vec3 operator *(const Mat &m, const Vec3 &v);
