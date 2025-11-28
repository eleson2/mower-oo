#ifndef INTEGERMATH_H
#define INTEGERMATH_H

#include "globals.hpp"
#include <FixedTrig.hpp>

// ============================================================================
// FixedTrig Wrapper for Mower Navigation
// ============================================================================
// This file provides the same API as the old IntegerMath.h but uses the
// high-performance FixedTrig library internally for better accuracy.
//
// FixedTrig uses:
// - Angle range: 0-16383 (14-bit, representing 0-360°)
// - Output scale: ±8192 for sin/cos (representing ±1.0)
//
// Mower uses:
// - Angle range: 0-3599 (tenths of degrees, 0.0° to 359.9°)
// - Output scale: ±1000 for sin/cos (representing ±1.0)
//
// This wrapper handles the conversion between the two formats.
// ============================================================================

// Use 128-entry lookup tables (good balance of speed/memory for AVR)
// Memory usage: ~768 bytes in PROGMEM (flash), not RAM!
using Trig = FastTrigOptimized<128, 128, 128>;

// ============================================================================
// ANGLE CONVERSION HELPERS
// ============================================================================

// Convert mower angle (tenths of degrees, 0-3599) to FixedTrig angle (0-16383)
// Formula: fixedAngle = mowerAngle * 16384 / 3600
// Optimized: fixedAngle = (mowerAngle * 4551) >> 10  (4551/1024 ≈ 16384/3600)
inline uint16_t mowerToFixedAngle(angle_t angle) {
    // Normalize first
    while (angle >= ANGLE_360) angle -= ANGLE_360;
    while (angle < 0) angle += ANGLE_360;

    // Convert using fixed-point multiplication
    // 16384 / 3600 = 4.551111... ≈ 4551/1000
    return (uint16_t)(((int32_t)angle * 4551) / 1000);
}

// Convert FixedTrig angle (0-16383) to mower angle (0-3599)
// Formula: mowerAngle = fixedAngle * 3600 / 16384
// Optimized: mowerAngle = (fixedAngle * 219) >> 10  (219/1024 ≈ 3600/16384)
inline angle_t fixedToMowerAngle(uint16_t fixedAngle) {
    return (angle_t)(((int32_t)fixedAngle * 219) >> 10);
}

// Convert FixedTrig output (±8192) to mower scale (±1000)
// Formula: mowerValue = fixedValue * 1000 / 8192
// Optimized: mowerValue = (fixedValue * 125) >> 10  (125/1024 ≈ 1000/8192)
inline int16_t fixedToMowerScale(int16_t fixedValue) {
    return (int16_t)(((int32_t)fixedValue * 125) >> 10);
}

// ============================================================================
// TRIGONOMETRIC FUNCTIONS (Compatible with old API)
// ============================================================================

// Integer atan2 - returns angle in tenths of degrees (0-3599)
// Now uses FixedTrig's optimized CORDIC algorithm
inline angle_t atan2_int(int32_t y, int32_t x) {
    // Handle special case
    if (x == 0 && y == 0) return 0;

    // Scale down if needed to prevent overflow in FixedTrig
    // FixedTrig expects int16_t inputs
    while (y > 32767 || y < -32767 || x > 32767 || x < -32767) {
        y >>= 1;
        x >>= 1;
    }

    // Call FixedTrig (returns 0-16383)
    uint16_t fixedAngle = Trig::atan2(static_cast<int16_t>(y), static_cast<int16_t>(x));

    // Convert to mower format (0-3599)
    return fixedToMowerAngle(fixedAngle);
}

// Normalize angle to 0-3599 range
inline angle_t normalizeAngle(angle_t angle) {
    while (angle >= ANGLE_360) angle -= ANGLE_360;
    while (angle < 0) angle += ANGLE_360;
    return angle;
}

// Calculate angle difference (shortest path)
// Returns signed angle in tenths of degrees (-1800 to +1800)
inline int16_t angleDifference(angle_t target, angle_t current) {
    int16_t diff = target - current;

    // Normalize to ±180° (±1800 tenths)
    while (diff > ANGLE_180) diff -= ANGLE_360;
    while (diff < -ANGLE_180) diff += ANGLE_360;

    return diff;
}

// Integer sine - input: tenths of degrees (0-3599), output: scaled by 1000
// sin(angle) * 1000, so sin(90°) = 1000
// Now uses FixedTrig's optimized lookup table with interpolation
inline int16_t sin_int(angle_t angle) {
    // Convert mower angle to FixedTrig angle
    uint16_t fixedAngle = mowerToFixedAngle(angle);

    // Call FixedTrig (returns ±8192)
    int16_t fixedResult = Trig::sin(fixedAngle);

    // Convert to mower scale (±1000)
    return fixedToMowerScale(fixedResult);
}

// Integer cosine - input: tenths of degrees (0-3599), output: scaled by 1000
// cos(angle) * 1000, so cos(0°) = 1000
// Now uses FixedTrig's optimized lookup table with interpolation
inline int16_t cos_int(angle_t angle) {
    // Convert mower angle to FixedTrig angle
    uint16_t fixedAngle = mowerToFixedAngle(angle);

    // Call FixedTrig (returns ±8192)
    int16_t fixedResult = Trig::cos(fixedAngle);

    // Convert to mower scale (±1000)
    return fixedToMowerScale(fixedResult);
}

// ============================================================================
// LEGACY COMPATIBILITY (optional - can be removed if not used)
// ============================================================================

// sin_lookup and cos_lookup are now just aliases to sin_int/cos_int
// The old 45° lookup table has been replaced by FixedTrig's 128-entry table
inline int16_t sin_lookup(angle_t angle) {
    return sin_int(angle);
}

inline int16_t cos_lookup(angle_t angle) {
    return cos_int(angle);
}

// ============================================================================
// ADDITIONAL UTILITIES FROM FIXEDTRIG
// ============================================================================

// Fast integer square root (better than the one in GeometryUtils)
inline uint32_t fast_sqrt(uint32_t x) {
    return Trig::fast_sqrt(x);
}

// Fast vector magnitude using CORDIC (no multiplication overhead)
// Returns: sqrt(x² + y²)
inline int32_t fast_magnitude(int32_t x, int32_t y) {
    // Scale down if needed
    while (x > 32767 || x < -32767 || y > 32767 || y < -32767) {
        x >>= 1;
        y >>= 1;
        return Trig::magnitude(x, y) << 1;
    }
    return Trig::magnitude(x, y);
}

#endif // INTEGERMATH_H
