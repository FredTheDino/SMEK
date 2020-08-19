#include "smek_quat.h"
#include "smek_mat4.h"
#include "../test.h"

i32 format(char *buffer, u32 size, FormatHint args, H q) {
    return snprintf(buffer, size, "(w%0*.*f, %0*.*f, %0*.*f, %0*.*f)",
                    args.num_zero_pad, args.num_decimals, q.w,
                    args.num_zero_pad, args.num_decimals, q.x,
                    args.num_zero_pad, args.num_decimals, q.y,
                    args.num_zero_pad, args.num_decimals, q.z);
}

H H::operator-() {
    return { -x, -y, -z, w };
}

H H::operator+(H h) {
    return { x + h.x, y + h.y, z + h.z, w + h.w };
}

H H::operator-(H h) {
    return { x - h.x, y - h.y, z - h.z, w - h.w };
}

void H::operator+=(H h) {
    x += h.x;
    y += h.y;
    z += h.z;
    w += h.w;
}

void H::operator-=(H h) {
    x -= h.x;
    y -= h.y;
    z -= h.z;
    w -= h.w;
}

H H::operator*(real s) {
    return { x * s, y * s, z * s, w * s };
}

H H::operator/(real s) {
    real d = 1.0f / s;
    return (*this) * d;
}

H H::operator*(H o) {
    H result = {};
    result.v = cross(v, o.v) + o.v * w + v * o.w;
    result.w = w * o.w - dot(v, o.v);
    return result;
}

void H::operator*=(H o) {
    *this = (*this) * (o);
}

H H::operator/(H o) {
    return (*this) * (-o);
}

void H::operator/=(H o) {
    *this = (*this) / o;
}

Vec3 H::operator*(Vec3 v) {
    H p = { v.x, v.y, v.z, 0.0f };
    H tmp = (*this) * p;
    tmp = tmp * -(*this);
    return { tmp.x, tmp.y, tmp.z };
}

Vec3 H::operator/(Vec3 v) {
    H p = { v.x, v.y, v.z, 0.0f };
    H h = -(*this);
    H tmp = h * p;
    tmp = tmp * -h;
    return { tmp.x, tmp.y, tmp.z };
}

real length_squared(H h) {
    return h.x * h.x + h.y * h.y + h.z * h.z + h.w * h.w;
}

real length(H h) {
    return Math::sqrt(length_squared(h));
}

H normalized(H h) {
    return h / length(h);
}

H conjugate(H h) {
    return { -h.x, -h.y, -h.z, h.w };
}

H lerp(H q1, H q2, real lerp) {
    q1 = normalized(q1);
    q2 = normalized(q2);
    if (dot(q1, q2) < 0) {
        q1 = -q1;
    }
    return normalized(q2 * lerp + (q1 * (1.0f - lerp)));
}

H H::from(real roll, real pitch, real yaw) {
    real cy = Math::cos(yaw * 0.5);
    real sy = Math::sin(yaw * 0.5);
    real cr = Math::cos(roll * 0.5);
    real sr = Math::sin(roll * 0.5);
    real cp = Math::cos(pitch * 0.5);
    real sp = Math::sin(pitch * 0.5);

    return {
        cy * sr * cp - sy * cr * sp,
        cy * cr * sp + sy * sr * cp,
        sy * cr * cp - cy * sr * sp,
        cy * cr * cp + sy * sr * sp
    };
}

H H::from(Vec3 axis, real angle) {
    real theta = angle / 2.0f;
    H result = {};
    result.v = axis * Math::sin(theta);
    result.w = (real)Math::cos(theta);
    result = normalized(result);
    return result;
}

Vec3 to_euler(H h) {
    // Roll
    real roll = Math::atan2(1.0f - 2.0f * (h.x * h.x + h.y * h.y), 2.0f * (h.w * h.x + h.y * h.z));

    // Pitch
    real sinp = 2.0f * (h.w * h.y - h.z * h.x);
    real pitch;
    if (Math::abs(sinp) >= 1.0f)
        pitch = Math::copysign(PI / 2.0f, sinp);
    else
        pitch = Math::asin(sinp);

    // Yaw
    real yaw = Math::atan2(1.0f - 2.0f * (h.y * h.y + h.z * h.z), 2.0f * (h.w * h.z + h.x * h.y));
    return { roll, pitch, yaw };
}

Mat Mat::from(H q) {
    return Mat::from(
        1.0f - 2.0f * q.y * q.y - 2.0f * q.z * q.z,
        2.0f * q.x * q.y - 2.0f * q.w * q.z,
        2.0f * q.w * q.y + 2.0f * q.x * q.z,
        0.0f,

        2.0f * q.x * q.y + 2.0f * q.w * q.z,
        1.0f - 2.0f * q.x * q.x - 2.0f * q.z * q.z,
        2.0f * q.y * q.z - 2.0f * q.w * q.x,
        0.0f,

        2.0f * q.x * q.z - 2.0f * q.w * q.y,
        2.0f * q.w * q.x + 2.0f * q.y * q.z,
        1.0f - 2.0f * q.x * q.x - 2.0f * q.y * q.y,
        0.0f,

        0.0f,
        0.0f,
        0.0f,
        1.0f);
};
