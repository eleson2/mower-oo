#ifndef MOWER_TYPES_H
#define MOWER_TYPES_H

#include <stdint.h>

// Mower-specific type definitions
// These types are used throughout the mower application

// Integer-only units for sensors (no floats!)
typedef int16_t angle_t;      // Angle in tenths of degrees (0-3599 = 0.0° to 359.9°)
typedef int32_t distance_t;   // Distance in millimeters
typedef uint32_t time_ms_t;   // Time in milliseconds

// Forward declare for use in Point2D_int methods
namespace IntegerMath {
    int64_t integerSqrt64(int64_t n);
}

// Simple 2D point structure for GPS coordinates
// INTEGER ONLY - distances in millimeters!
struct Point2D_int {
    distance_t x;  // X coordinate in millimeters
    distance_t y;  // Y coordinate in millimeters

    Point2D_int() : x(0), y(0) {}
    Point2D_int(distance_t _x, distance_t _y) : x(_x), y(_y) {}

    // Vector operations
    Point2D_int operator-(const Point2D_int& other) const {
        return Point2D_int(x - other.x, y - other.y);
    }

    Point2D_int operator+(const Point2D_int& other) const {
        return Point2D_int(x + other.x, y + other.y);
    }

    Point2D_int operator*(int32_t scalar) const {
        return Point2D_int((x * scalar), (y * scalar));
    }

    Point2D_int operator/(int32_t scalar) const {
        return Point2D_int(x / scalar, y / scalar);
    }

    bool operator==(const Point2D_int& other) const {
        return x == other.x && y == other.y;
    }

    bool operator!=(const Point2D_int& other) const {
        return !(*this == other);
    }

    // Dot product
    int64_t dot(const Point2D_int& other) const {
        return (int64_t)x * other.x + (int64_t)y * other.y;
    }

    // Cross product (2D returns scalar)
    int64_t cross(const Point2D_int& other) const {
        return (int64_t)x * other.y - (int64_t)y * other.x;
    }

    // Magnitude (in mm)
    distance_t magnitude() const {
        int64_t magSquared = (int64_t)x * x + (int64_t)y * y;
        return IntegerMath::integerSqrt64(magSquared);
    }

    // Calculate distance to another point (returns mm)
    distance_t distanceTo(const Point2D_int& other) const {
        int32_t dx = x - other.x;
        int32_t dy = y - other.y;
        int64_t distSquared = (int64_t)dx * dx + (int64_t)dy * dy;
        return IntegerMath::integerSqrt64(distSquared);
    }

    // Normalize to unit vector (scaled by 1000 for precision)
    // Returns vector scaled so magnitude = 1000
    Point2D_int normalized() const {
        distance_t mag = magnitude();
        if (mag > 10) {  // Avoid division by very small numbers
            return Point2D_int((x * 1000) / mag, (y * 1000) / mag);
        }
        return Point2D_int(0, 0);
    }
};

// Angle constants
constexpr angle_t ANGLE_0   = 0;
constexpr angle_t ANGLE_90  = 900;   // 90.0 degrees = 900 tenths
constexpr angle_t ANGLE_180 = 1800;  // 180.0 degrees = 1800 tenths
constexpr angle_t ANGLE_270 = 2700;  // 270.0 degrees = 2700 tenths
constexpr angle_t ANGLE_360 = 3600;  // 360.0 degrees = 3600 tenths

// Helper macros for angle conversion
#define DEGREES_TO_ANGLE(deg) ((angle_t)((deg) * 10))
#define ANGLE_TO_DEGREES(ang) ((ang) / 10)

// Helper macros for distance conversion
#define METERS_TO_MM(m) ((distance_t)((m) * 1000))
#define MM_TO_METERS(mm) ((mm) / 1000)

#endif // MOWER_TYPES_H
