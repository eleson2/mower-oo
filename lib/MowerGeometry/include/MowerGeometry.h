#ifndef MOWER_GEOMETRY_H
#define MOWER_GEOMETRY_H

#include "MowerTypes.h"
#include "IntegerMathUtils.h"

// Mower-specific geometry utilities
// These functions work with mower domain types (Point2D_int, angle_t, distance_t)
// and implement application-specific conventions and behaviors

namespace MowerGeometry {

// ============================================================================
// DISTANCE CALCULATIONS (using Point2D_int)
// ============================================================================

// Calculate Euclidean distance between two points
// Returns: distance in millimeters
inline distance_t distanceBetweenPoints(const Point2D_int& p1, const Point2D_int& p2) {
    int32_t dx = p2.x - p1.x;
    int32_t dy = p2.y - p1.y;
    return IntegerMath::vectorLength(dx, dy);
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
inline distance_t distanceToLineSegment(const Point2D_int& point,
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

    return IntegerMath::vectorLength(distX, distY);
}

// ============================================================================
// ANGLE CALCULATIONS (using angle_t - tenths of degrees)
// ============================================================================

// Calculate angle between two vectors (in tenths of degrees)
// Returns: angle 0-3599 (0.0° to 359.9°)
// Uses approximation for speed
inline angle_t angleBetweenVectors(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    // Normalize both vectors
    int32_t norm1X, norm1Y;
    int32_t norm2X, norm2Y;

    IntegerMath::normalizeVector(x1, y1, norm1X, norm1Y);
    IntegerMath::normalizeVector(x2, y2, norm2X, norm2Y);

    // Dot product = cos(angle) * 1000
    int32_t cosAngle = IntegerMath::dotProduct(norm1X, norm1Y, norm2X, norm2Y);

    // Approximate acos using polynomial (good enough for most cases)
    // acos(x) ≈ π/2 - x (for small angles)
    // For better accuracy, use lookup table or more complex polynomial

    // Simple approximation: angle ≈ 900 - (cosAngle * 900 / 1000)
    angle_t angle = 900 - (cosAngle * 900) / 1000;

    return angle;
}

// ============================================================================
// PERPENDICULAR AND PROJECTION (using Point2D_int)
// ============================================================================

// Calculate perpendicular vector (rotated 90° left/right)
// direction: true = left (CCW), false = right (CW)
inline void getPerpendicular(int32_t x, int32_t y, bool leftSide,
                             int32_t& perpX, int32_t& perpY) {
    if (leftSide) {
        IntegerMath::rotateCCW90(x, y, perpX, perpY);  // Left: (-y, x)
    } else {
        IntegerMath::rotateCW90(x, y, perpX, perpY);   // Right: (y, -x)
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
// BOUNDING BOX (using Point2D_int)
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
// INTERPOLATION (using Point2D_int)
// ============================================================================

// Linear interpolation between two points
// t is scaled by 1000 (0-1000 represents 0.0-1.0)
inline Point2D_int lerpPoint(const Point2D_int& a, const Point2D_int& b, int32_t t) {
    Point2D_int result;
    result.x = IntegerMath::lerp(a.x, b.x, t);
    result.y = IntegerMath::lerp(a.y, b.y, t);
    return result;
}

} // namespace MowerGeometry

#endif // MOWER_GEOMETRY_H
