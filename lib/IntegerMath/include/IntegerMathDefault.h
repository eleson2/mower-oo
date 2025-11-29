#ifndef INTEGERMATH_DEFAULT_H
#define INTEGERMATH_DEFAULT_H

#include "FixedTrig.hpp"
#include "IntegerMathGeneric.h"

// Default FixedTrig-backed instantiation of IntegerTrigWrapper.
// This is generic integer trigonometry (no mower-specific logic).
using DefaultTrig = FastTrigOptimized<128, 128, 128>;
using IntegerTrig = IntegerTrigWrapper<DefaultTrig, int16_t, 3600, 1800, 1000, 8192, 16384>;

// Free-function API (generic integer trig wrappers)
inline uint16_t mowerToFixedAngle(IntegerTrig::angle_t angle) { return IntegerTrig::mowerToFixedAngle(angle); }
inline IntegerTrig::angle_t fixedToMowerAngle(uint16_t a) { return IntegerTrig::fixedToMowerAngle(a); }
inline int16_t fixedToMowerScale(int16_t v) { return IntegerTrig::fixedToMowerScale(v); }

inline IntegerTrig::angle_t atan2_int(int32_t y, int32_t x) { return IntegerTrig::atan2_int(y, x); }
inline IntegerTrig::angle_t normalizeAngle(IntegerTrig::angle_t angle) { return IntegerTrig::normalizeAngle_wrap(angle); }
inline int16_t angleDifference(IntegerTrig::angle_t target, IntegerTrig::angle_t current) { return IntegerTrig::angleDifference(target, current); }

inline int16_t sin_int(IntegerTrig::angle_t angle) { return IntegerTrig::sin_int(angle); }
inline int16_t cos_int(IntegerTrig::angle_t angle) { return IntegerTrig::cos_int(angle); }
inline int16_t sin_lookup(IntegerTrig::angle_t angle) { return IntegerTrig::sin_lookup(angle); }
inline int16_t cos_lookup(IntegerTrig::angle_t angle) { return IntegerTrig::cos_lookup(angle); }

inline uint32_t fast_sqrt(uint32_t x) { return IntegerTrig::fast_sqrt(x); }
inline int32_t fast_magnitude(int32_t x, int32_t y) { return IntegerTrig::fast_magnitude(x, y); }

#endif // INTEGERMATH_DEFAULT_H
