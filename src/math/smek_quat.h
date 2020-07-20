#pragma once
#include "smek_vec.h"

struct H;
H normalized(H h);

struct H {
	union {
		struct {
			real x, y, z, w;
		};
        Vec3 v;
		real _[4];
	};

    H(real x=0.0, real y=0.0, real z=0.0, real w=1.0): x(x), y(y), z(z), w(w) {}

	H operator- ();

	H operator+ (H h);
	H operator- (H h);

	void operator+= (H h);
	void operator-= (H h);

	H operator* (real s);
	H operator/ (real s);

	H operator* (H o);
	void operator*= (H o);

	H operator/ (H o);
	void operator/= (H o);

	Vec3 operator* (Vec3 v);
	Vec3 operator/ (Vec3 v);

    static H from(real roll, real pitch, real yaw);

    static H from(Vec3 axis, real angle);
};

///*
real length_squared(H h);

///*
real length(H h);

///*
H normalized(H h);

///*
H conjugate(H h);

///*
H lerp(H q1, H q2, real lerp);

///*
Vec3 to_euler(H h);

i32 format(char *buffer, u32 size, FormatHint args, H q);

using Quat = H;

