#pragma once

#include <Defs.hpp>
#include <stdint.h>
#include <niLang/STL/string.h>
#include <niLang/STL/limits.h>

namespace spark {
namespace common {

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;
union uf32 {
    uf32(f32 f = 0.0f) : f_(f) {}

    // Portable extraction of components.
    bool negative() const { return i_ < 0; }
    i32 rawMantissa() const { return i_ & ((1 << 23) - 1); }
    i32 rawExponent() const { return (i_ >> 23) & 0xFF; }

    i32 i_;
    float f_;
#ifdef skDebug
    struct {
        // Bitfields for exploration. Do not use in production code.
        u32 mantissa : 23;
        u32 exponent : 8;
        u32 sign : 1;
    } parts;
#endif
};

#define skMin(a,b) (a < b ? a : b)
#define skMax(a,b) (a > b ? a : b)
#define skClamp(v,min,max) skMin(skMax(v, min), max)
#define skNearest(f) astl::round(f)
constexpr f32 skEpsilon = astl::numeric_limits<f32>::epsilon();
constexpr f32 skEpsilonL = skEpsilon * 4;
#define skUndefinedU 0xDEADBEEF
#define skUndefinedI static_cast<i32>(skUndefinedU)
#define skBit(bit) 1ll << (bit)
#define skBitOn(flag,bit) (static_cast<u64>(flag) | static_cast<u64>(bit))
#define skBitOff(flag,bit) (static_cast<u64>(flag) & ~static_cast<u64>(bit))
#define skMask(flag,bit) (static_cast<u64>(flag) & static_cast<u64>(bit))
#define skHasBit(flag,bit) (skMask(flag,bit) == static_cast<u64>(bit))
inline u64 skSecsToNs(f64 secs) { return secs * 1000000000ULL; }
inline f64 skNsToSecs(u64 ns) { return ns / 1000000000ULL; }

inline bool skAlmostEqualF(f32 a, f32 b, f32 epsilon = skEpsilonL) {
    return ni::Abs(a-b) <= epsilon;
}
inline bool skAlmostEqualUlpF(f32 a, f32 b, i32 maxUlpsDiff = 1) {
    // If close enough using machine epsilon,
    // consider it equal right away.
    if (skAlmostEqualF(a, b, skEpsilon)) {
        return true;
    }

    const uf32 ua(a);
    const uf32 ub(b);

    // Different signs means they do not match.
    if (ua.negative() != ub.negative())
        return a == b; // Check for equality to make sure +0==-0

    // Find the difference in ULPs.
    const i32 ulpsDiff = abs(ua.i_ - ub.i_);
    return ulpsDiff <= maxUlpsDiff;
}

template <typename T> T skUndefined();

template <>
inline u32 skUndefined<u32>() {
    return skUndefinedU;
}

template <>
inline i32 skUndefined<i32>() {
    return skUndefinedI;
}

} }; // namespace spark::ui
