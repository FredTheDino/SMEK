#pragma once
#include <cstdint>

///# Game standard types
// Use these if you can, since the size is always known,
// and the look sexy. (They're also easy to change if we
// have to).

///* b8
using b8 = uint8_t;

///* u8
using u8 = uint8_t;

///* u16
using u16 = uint16_t;

///* u32
using u32 = uint32_t;

///* u64
using u64 = uint64_t;

///* i8
using i8 = int8_t;

///* i16
using i16 = int16_t;

///* i32
using i32 = int32_t;

///* i64
using i64 = int64_t;

///* f32
using f32 = float;

///* f64
using f64 = double;

///* real
using real = f32;

///* PI
constexpr real PI = 3.1415926535;
///* E
constexpr real E = 2.7182818284;

///* AssetID
// A way to identify assets.
//
// The string is expected to be const so it potentially can be hashed during
// compile time.
struct AssetID {
    AssetID(const char *);
    AssetID(u64 id)
        : id(id) {}
    AssetID()
        : id(NONE()) {}

    static AssetID NONE() { return 0xFFFFFFFF; }

    u64 id;

    bool operator==(AssetID &other) const {
        return id == other;
    }

    operator u64() const {
        return id;
    }
};

