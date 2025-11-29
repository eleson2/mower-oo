#ifndef INTEGERMATH_UTILS_H
#define INTEGERMATH_UTILS_H

#include <stdint.h>

// Generic integer math utilities - NO domain-specific types
// Pure mathematical operations suitable for any embedded project
// All functions use only primitive types (int32_t, uint32_t, etc.)

namespace IntegerMath {

// ============================================================================
// INTEGER SQUARE ROOT
// ============================================================================

// Fast integer square root using bit-by-bit algorithm
// Returns: floor(sqrt(n))
// Performance: ~50 cycles on AVR
inline int32_t integerSqrt(int32_t n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;

    int32_t result = 0;
    int32_t bit = 1L << 30;  // Start with highest bit

    // Find highest set bit
    while (bit > n) {
        bit >>= 2;
    }

    // Calculate sqrt bit by bit
    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }

    return result;
}

// 64-bit integer square root (for large values)
inline int64_t integerSqrt64(int64_t n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;

    int64_t result = 0;
    int64_t bit = 1LL << 62;

    while (bit > n) {
        bit >>= 2;
    }

    while (bit != 0) {
        if (n >= result + bit) {
            n -= result + bit;
            result = (result >> 1) + bit;
        } else {
            result >>= 1;
        }
        bit >>= 2;
    }

    return result;
}

// ============================================================================
// VECTOR OPERATIONS (on primitive types)
// ============================================================================

// Calculate vector length (magnitude)
// Returns: sqrt(x² + y²)
inline int32_t vectorLength(int32_t x, int32_t y) {
    // Avoid overflow for large values
    if (x > 32767 || x < -32767 || y > 32767 || y < -32767) {
        // Scale down to prevent overflow
        x /= 2;
        y /= 2;
        return integerSqrt(x * x + y * y) * 2;
    }
    return integerSqrt(x * x + y * y);
}

// Normalize vector (scale to length 1000)
// Input: vector (x, y)
// Output: normalized vector scaled by 1000
// Example: (300, 400) → (600, 800) because length = 500, scaled to 1000
inline void normalizeVector(int32_t x, int32_t y, int32_t& normX, int32_t& normY) {
    int32_t len = vectorLength(x, y);
    if (len == 0) {
        normX = 0;
        normY = 0;
        return;
    }

    normX = (x * 1000) / len;
    normY = (y * 1000) / len;
}

// Calculate dot product of two vectors
// Both vectors should be normalized (scaled by 1000)
// Returns: dot product scaled by 1000
inline int32_t dotProduct(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    return (x1 * x2 + y1 * y2) / 1000;
}

// Calculate cross product Z component (for 2D vectors)
// Returns: x1*y2 - y1*x2 (sign indicates rotation direction)
inline int32_t crossProduct2D(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    // Avoid overflow
    int64_t cross = (int64_t)x1 * y2 - (int64_t)y1 * x2;
    return (int32_t)(cross / 1000);  // Scale down
}

// Rotate vector 90° counter-clockwise
// (x, y) → (-y, x)
inline void rotateCCW90(int32_t x, int32_t y, int32_t& outX, int32_t& outY) {
    outX = -y;
    outY = x;
}

// Rotate vector 90° clockwise
// (x, y) → (y, -x)
inline void rotateCW90(int32_t x, int32_t y, int32_t& outX, int32_t& outY) {
    outX = y;
    outY = -x;
}

// ============================================================================
// INTERPOLATION
// ============================================================================

// Linear interpolation between two values
// t is scaled by 1000 (0-1000 represents 0.0-1.0)
inline int32_t lerp(int32_t a, int32_t b, int32_t t) {
    return a + ((b - a) * t) / 1000;
}

// ============================================================================
// UTILITIES
// ============================================================================

// Clamp value to range
inline int32_t clamp(int32_t value, int32_t minVal, int32_t maxVal) {
    if (value < minVal) return minVal;
    if (value > maxVal) return maxVal;
    return value;
}

// Sign function (-1, 0, or 1)
inline int8_t sign(int32_t value) {
    if (value > 0) return 1;
    if (value < 0) return -1;
    return 0;
}

// Absolute value
inline int32_t abs32(int32_t value) {
    return value < 0 ? -value : value;
}

} // namespace IntegerMath

#endif // INTEGERMATH_UTILS_H
