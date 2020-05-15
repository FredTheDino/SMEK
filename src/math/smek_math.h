#include "types.h"

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

