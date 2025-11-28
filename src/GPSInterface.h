#ifndef GPSINTERFACE_H
#define GPSINTERFACE_H

#include "globals.hpp"
#include "Arduino.h"

// Simple 2D point structure for GPS coordinates
// INTEGER ONLY - distances in millimeters!
struct Point2D_int {
    distance_t x;  // X coordinate in millimeters
    distance_t y;  // Y coordinate in millimeters

    Point2D_int() : x(0), y(0) {}
    Point2D_int(distance_t _x, distance_t _y) : x(_x), y(_y) {}

    // Calculate distance to another point (returns mm)
    distance_t distanceTo(const Point2D_int& other) const {
        int32_t dx = x - other.x;
        int32_t dy = y - other.y;

        // Integer square root approximation
        // sqrt(dx² + dy²)
        int64_t distSquared = (int64_t)dx * dx + (int64_t)dy * dy;
        return isqrt64(distSquared);
    }

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
        return isqrt64(magSquared);
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

private:
    // Integer square root using Newton's method
    // Fast approximation for 64-bit integers
    static distance_t isqrt64(int64_t n) {
        if (n <= 0) return 0;
        if (n == 1) return 1;

        int64_t x = n;
        int64_t y = (x + 1) / 2;

        while (y < x) {
            x = y;
            y = (x + n / x) / 2;
        }
        return (distance_t)x;
    }
};

// GPS Interface stub - to be replaced with actual GPS implementation
// INTEGER ONLY - positions in millimeters!
class GPSInterface {
private:
    Point2D_int _currentPosition;
    bool _hasFixSimulated;

public:
    GPSInterface() : _currentPosition(0, 0), _hasFixSimulated(false) {}

    // Initialize GPS module
    void begin() {
        // TODO: Initialize actual GPS hardware
        _hasFixSimulated = false;
    }

    // Update GPS reading (call periodically)
    void update() {
        // TODO: Read from actual GPS module
        // Convert lat/lon to local coordinates in millimeters
        _hasFixSimulated = true;
    }

    // Check if GPS has valid fix
    bool hasFix() const {
        return _hasFixSimulated;
    }

    // Get current position (in millimeters)
    Point2D_int getPosition() const {
        return _currentPosition;
    }

    // Stub: Set position manually in millimeters (for testing)
    void setPositionMM(distance_t x, distance_t y) {
        _currentPosition.x = x;
        _currentPosition.y = y;
        _hasFixSimulated = true;
    }

    // Stub: Set position in meters (for convenience)
    void setPositionMeters(int32_t xMeters, int32_t yMeters) {
        _currentPosition.x = METERS_TO_MM(xMeters);
        _currentPosition.y = METERS_TO_MM(yMeters);
        _hasFixSimulated = true;
    }

    // Stub: Set position with fractional meters (for testing)
    // m_tenths: meters in tenths (e.g., 15 = 1.5m = 1500mm)
    void setPositionTenthsOfMeters(int32_t xTenths, int32_t yTenths) {
        _currentPosition.x = xTenths * 100;  // tenths of meter to mm
        _currentPosition.y = yTenths * 100;
        _hasFixSimulated = true;
    }

    // Get number of satellites (stub)
    int getSatellites() const {
        return _hasFixSimulated ? 8 : 0;
    }

    // Get HDOP (horizontal dilution of precision) - stub
    int getHDOP() const {
        return _hasFixSimulated ? 12 : 999;  // 1.2 → 12 (tenths)
    }
};

// Type alias for compatibility
using Point2D = Point2D_int;

#endif
