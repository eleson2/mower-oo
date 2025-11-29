#ifndef INTEGERMATH_GENERIC_H
#define INTEGERMATH_GENERIC_H

#include <stdint.h>

// Generic, header-only templated IntegerMath implementation.
// Does not assume any particular trig implementation; the TrigT
// template parameter must provide the fixed-trig API (sin/cos/atan2/etc.).
template <
    typename TrigT,
    typename AngleT = int16_t,
    AngleT ANGLE_360 = static_cast<AngleT>(3600),
    AngleT ANGLE_180 = static_cast<AngleT>(1800),
    int32_t MOWER_SCALE = 1000,
    int32_t FIXED_SCALE = 8192,
    int32_t FIXED_ANGLE_MAX = 16384
>
struct IntegerTrigWrapper {
    using trig_t = TrigT;
    using angle_t = AngleT;

    // Use wider intermediate types for compile-time arithmetic.
    static constexpr int64_t mower_to_fixed_num64 = (static_cast<int64_t>(FIXED_ANGLE_MAX) * 1000000LL) / static_cast<int64_t>(ANGLE_360);
    static constexpr int32_t mower_to_fixed_num = static_cast<int32_t>(mower_to_fixed_num64);
    static constexpr int32_t mower_to_fixed_div = 1000000;

    static constexpr int64_t fixed_to_mower_mul64 = (static_cast<int64_t>(ANGLE_360) * 1024LL) / static_cast<int64_t>(FIXED_ANGLE_MAX);
    static constexpr int32_t fixed_to_mower_mul = static_cast<int32_t>(fixed_to_mower_mul64);
    static constexpr int fixed_to_mower_shift = 10;

    static constexpr int64_t fixed_to_mower_scale_mul64 = (static_cast<int64_t>(MOWER_SCALE) * 1024LL) / static_cast<int64_t>(FIXED_SCALE);
    static constexpr int32_t fixed_to_mower_scale_mul = static_cast<int32_t>(fixed_to_mower_scale_mul64);
    static constexpr int fixed_to_mower_scale_shift = 10;

    // Normalize angle to 0..ANGLE_360-1 without division
    static inline angle_t normalizeAngle(angle_t angle) {
        int32_t a = static_cast<int32_t>(angle);
        if (a >= 0 && a < ANGLE_360) return static_cast<angle_t>(a);
        while (a >= ANGLE_360) a -= ANGLE_360;
        while (a < 0) a += ANGLE_360;
        return static_cast<angle_t>(a);
    }

    static inline uint16_t mowerToFixedAngle(angle_t angle) {
        angle = normalizeAngle(angle);
        // Approximate multiplier using multiply+shift to avoid division
        return static_cast<uint16_t>((static_cast<int32_t>(angle) * 4663) >> 10);
    }

    static inline angle_t fixedToMowerAngle(uint16_t fixedAngle) {
        return static_cast<angle_t>((static_cast<int32_t>(fixedAngle) * fixed_to_mower_mul) >> fixed_to_mower_shift);
    }

    static inline int16_t fixedToMowerScale(int16_t fixedValue) {
        return static_cast<int16_t>((static_cast<int32_t>(fixedValue) * fixed_to_mower_scale_mul) >> fixed_to_mower_scale_shift);
    }

    // atan2: returns angle in mower units (tenths of degree)
    static inline angle_t atan2_int(int32_t y, int32_t x) {
        if (x == 0 && y == 0) return static_cast<angle_t>(0);
        while (y > 32767 || y < -32767 || x > 32767 || x < -32767) {
            y >>= 1;
            x >>= 1;
        }
        uint16_t fixedAngle = TrigT::atan2(static_cast<int16_t>(y), static_cast<int16_t>(x));
        return fixedToMowerAngle(fixedAngle);
    }

    static inline angle_t normalizeAngle_wrap(angle_t angle) { return normalizeAngle(angle); }

    static inline int16_t angleDifference(angle_t target, angle_t current) {
        int32_t diff = static_cast<int32_t>(target) - static_cast<int32_t>(current);
        while (diff > ANGLE_180) diff -= ANGLE_360;
        while (diff < -ANGLE_180) diff += ANGLE_360;
        return static_cast<int16_t>(diff);
    }

    static inline int16_t sin_int(angle_t angle) {
        uint16_t fixedAngle = mowerToFixedAngle(angle);
        int16_t fixedResult = TrigT::sin(fixedAngle);
        return fixedToMowerScale(fixedResult);
    }

    static inline int16_t cos_int(angle_t angle) {
        uint16_t fixedAngle = mowerToFixedAngle(angle);
        int16_t fixedResult = TrigT::cos(fixedAngle);
        return fixedToMowerScale(fixedResult);
    }

    static inline int16_t sin_lookup(angle_t angle) { return sin_int(angle); }
    static inline int16_t cos_lookup(angle_t angle) { return cos_int(angle); }

    static inline uint32_t fast_sqrt(uint32_t x) { return TrigT::fast_sqrt(x); }

    static inline int32_t fast_magnitude(int32_t x, int32_t y) {
        int shift = 0;
        int32_t tx = x, ty = y;
        while (tx > 32767 || tx < -32767 || ty > 32767 || ty < -32767) {
            tx >>= 1;
            ty >>= 1;
            ++shift;
        }
        int32_t mag = TrigT::magnitude(static_cast<int16_t>(tx), static_cast<int16_t>(ty));
        return static_cast<int32_t>(mag) << shift;
    }
};

#endif // INTEGERMATH_GENERIC_H
