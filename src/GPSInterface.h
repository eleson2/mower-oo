#ifndef GPSINTERFACE_H
#define GPSINTERFACE_H

#include "globals.hpp"
#include "Arduino.h"
#include "MowerTypes.h"
#include "MowerGeometry.h"
#include "IntegerMathUtils.h"

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
