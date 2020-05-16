#pragma once
#include "types.h"

namespace Math {

///*
// Returns the absolute value of a.
template<typename T>
T abs(T a);

///*
// Returns the minimum of the two values 'a' and 'b'.
template<typename T>
T min(T a, T b);

///*
// Returns the maximum of the two values 'a' and 'b'.
template<typename T>
T max(T a, T b);

///*
// Returns the 1 if a is larger or equal to zero, -1 otherwise.
template<typename T>
T sign(T a);

///*
// See sign, but returns 0 for 0.
template<typename T>
T sign_with_zero(T a);

///*
// Returns true if the value a lies within distance r of b.
template<typename T>
bool close_enough(T a, T b, f32 r=0.0001);

///*
// Raises a to the b power.
real pow(real a, real b);

///*
// Takes the square root of a.
real sqrt(real a);

///*
// Returns the module of a mod b.
real mod(real a, real b);

///*
// Returns the remained of a mod b. (Can be negative)
real rem(real a, real b);

///*
// Raises e to the a power.
real exp(real a);

///*
// Takes the logarithm base e of a.
real log(real a);

///*
// sin(a), a is in radians.
real sin(real a);

///*
// cos(a), a is in radians.
real cos(real a);

///*
// tan(a), a is in radians.
real tan(real a);

///*
// The inverse of the sin(x) function, returns radians.
real asin(real a);

///*
// The inverse of the sin(x) function, returns radians.
real acos(real a);

///*
// The inverse of the sin(x) function, returns radians.
real atan(real a);

///*
// The inverse of the sin(x) function, returns radians.
real atan2(real x, real y);

///*
// Returns a rounded up to the nearest integer.
template<typename R>
R cell(real a);

///*
// Returns a rounded down to the nearest integer.
template<typename R>
R floor(real a);

///*
// Returns a rounded to the nearest integer, 0.5 rounds down.
template<typename R>
R round(real a);

///*
// Converts radians to degrees.
real to_radians(real degrees);

///*
// Converts degrees to radians.
real to_degrees(real radians);

};
