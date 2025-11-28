#ifndef GEOMETRYUTILS_H
#define GEOMETRYUTILS_H

#include "globals.hpp"
#include "Arduino.h" 

// Common geometry utilities for integer-only math
// Used by: PerimeterOffset, PerimeterStorage, LineFollower, IntegerMath
// All functions use integer-only arithmetic - NO FLOATING POINT!

namespace GeometryUtils {

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

// ============================================================================
// VECTOR OPERATIONS
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
// DISTANCE CALCULATIONS
// ============================================================================

// Calculate Euclidean distance between two points
// Returns: distance in millimeters
inline int32_t distanceBetweenPoints(const Point2D_int& p1, const Point2D_int& p2) {
    int32_t dx = p2.x - p1.x;
    int32_t dy = p2.y - p1.y;
    return vectorLength(dx, dy);
}

// Calculate squared distance (faster, avoids sqrt)
// Returns: distance² in mm²
inline int32_t distanceSquared(const Point2D_int& p1, const Point2D_int& p2) {
    int32_t dx = p2.x - p1.x;
    int32_t dy = p2.y - p1.y;

    // Check for overflow
    if (dx > 46340 || dx < -46340 || dy > 46340 || dy < -46340) {
        // Scale down to prevent overflow (sqrt(2^31) ≈ 46340)
        dx /= 10;
        dy /= 10;
        return (dx * dx + dy * dy) * 100;
    }

    return dx * dx + dy * dy;
}

// Calculate distance from point to line segment
// Returns: perpendicular distance in mm
inline int32_t distanceToLineSegment(const Point2D_int& point,
                                      const Point2D_int& lineStart,
                                      const Point2D_int& lineEnd) {
    int32_t dx = lineEnd.x - lineStart.x;
    int32_t dy = lineEnd.y - lineStart.y;

    if (dx == 0 && dy == 0) {
        // Degenerate segment - distance to point
        return distanceBetweenPoints(point, lineStart);
    }

    // Project point onto line
    int32_t t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy);
    int32_t lenSq = dx * dx + dy * dy;

    int32_t projX, projY;

    if (t <= 0) {
        // Closest to lineStart
        projX = lineStart.x;
        projY = lineStart.y;
    } else if (t >= lenSq) {
        // Closest to lineEnd
        projX = lineEnd.x;
        projY = lineEnd.y;
    } else {
        // On segment
        projX = lineStart.x + (dx * t) / lenSq;
        projY = lineStart.y + (dy * t) / lenSq;
    }

    // Distance from point to projection
    int32_t distX = point.x - projX;
    int32_t distY = point.y - projY;

    return vectorLength(distX, distY);
}

// ============================================================================
// ANGLE CALCULATIONS
// ============================================================================

// Calculate angle between two vectors (in tenths of degrees)
// Returns: angle 0-3599 (0.0° to 359.9°)
// Uses approximation for speed
inline angle_t angleBetweenVectors(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    // Normalize both vectors
    int32_t norm1X, norm1Y;
    int32_t norm2X, norm2Y;

    normalizeVector(x1, y1, norm1X, norm1Y);
    normalizeVector(x2, y2, norm2X, norm2Y);

    // Dot product = cos(angle) * 1000
    int32_t cosAngle = dotProduct(norm1X, norm1Y, norm2X, norm2Y);

    // Approximate acos using polynomial (good enough for most cases)
    // acos(x) ≈ π/2 - x (for small angles)
    // For better accuracy, use lookup table or more complex polynomial

    // Simple approximation: angle ≈ 900 - (cosAngle * 900 / 1000)
    angle_t angle = 900 - (cosAngle * 900) / 1000;

    return angle;
}

// ============================================================================
// PERPENDICULAR AND PROJECTION
// ============================================================================

// Calculate perpendicular vector (rotated 90° left/right)
// direction: true = left (CCW), false = right (CW)
inline void getPerpendicular(int32_t x, int32_t y, bool leftSide,
                             int32_t& perpX, int32_t& perpY) {
    if (leftSide) {
        rotateCCW90(x, y, perpX, perpY);  // Left: (-y, x)
    } else {
        rotateCW90(x, y, perpX, perpY);   // Right: (y, -x)
    }
}

// Project point onto line (returns projection point)
inline Point2D_int projectPointOntoLine(const Point2D_int& point,
                                         const Point2D_int& lineStart,
                                         const Point2D_int& lineEnd) {
    int32_t dx = lineEnd.x - lineStart.x;
    int32_t dy = lineEnd.y - lineStart.y;

    if (dx == 0 && dy == 0) {
        return lineStart;  // Degenerate line
    }

    // t = dot(point - lineStart, lineEnd - lineStart) / |lineEnd - lineStart|²
    int32_t t = ((point.x - lineStart.x) * dx + (point.y - lineStart.y) * dy);
    int32_t lenSq = dx * dx + dy * dy;

    // Clamp t to [0, lenSq]
    if (t < 0) t = 0;
    if (t > lenSq) t = lenSq;

    Point2D_int projection;
    projection.x = lineStart.x + (dx * t) / lenSq;
    projection.y = lineStart.y + (dy * t) / lenSq;

    return projection;
}

// ============================================================================
// BOUNDING BOX
// ============================================================================

// Check if point is inside bounding box
inline bool isInsideBoundingBox(const Point2D_int& point,
                                 int32_t minX, int32_t maxX,
                                 int32_t minY, int32_t maxY) {
    return point.x >= minX && point.x <= maxX &&
           point.y >= minY && point.y <= maxY;
}

// Expand bounding box to include point
inline void expandBoundingBox(const Point2D_int& point,
                               int32_t& minX, int32_t& maxX,
                               int32_t& minY, int32_t& maxY) {
    if (point.x < minX) minX = point.x;
    if (point.x > maxX) maxX = point.x;
    if (point.y < minY) minY = point.y;
    if (point.y > maxY) maxY = point.y;
}

// ============================================================================
// INTERPOLATION
// ============================================================================

// Linear interpolation between two values
// t is scaled by 1000 (0-1000 represents 0.0-1.0)
inline int32_t lerp(int32_t a, int32_t b, int32_t t) {
    return a + ((b - a) * t) / 1000;
}

// Linear interpolation between two points
inline Point2D_int lerpPoint(const Point2D_int& a, const Point2D_int& b, int32_t t) {
    Point2D_int result;
    result.x = lerp(a.x, b.x, t);
    result.y = lerp(a.y, b.y, t);
    return result;
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

} // namespace GeometryUtils

#endif
