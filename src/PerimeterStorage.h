#ifndef PERIMETERSTORAGE_H
#define PERIMETERSTORAGE_H

#include "globals.hpp"
#include "Arduino.h"
#include "GeometryUtils.h"

// Maximum perimeter waypoints
#define MAX_PERIMETER_WAYPOINTS 1000

// Efficient perimeter storage using relative coordinates
// Stores waypoints as 16-bit offsets from previous point
// This allows ±32.767m range per segment while using only 4 bytes per waypoint
// Total memory: 1000 waypoints × 4 bytes = 4KB (vs 8KB for absolute coordinates)
class PerimeterStorage {
private:
    // Storage format: relative offsets in millimeters
    struct RelativeWaypoint {
        int16_t dx;  // Offset from previous X (mm, ±32767mm = ±32.7m)
        int16_t dy;  // Offset from previous Y (mm, ±32767mm = ±32.7m)
    };

    // First waypoint is absolute (reference point)
    Point2D_int _origin;

    // All subsequent waypoints are relative to previous
    RelativeWaypoint _waypoints[MAX_PERIMETER_WAYPOINTS - 1];

    int _waypointCount;

    // Bounding box (calculated from waypoints)
    int32_t _minX, _maxX;
    int32_t _minY, _maxY;
    bool _boundsValid;

public:
    PerimeterStorage() : _origin{0, 0}, _waypointCount(0),
                         _minX(0), _maxX(0), _minY(0), _maxY(0),
                         _boundsValid(false) {
    }

    // Add waypoint from absolute coordinates
    // Returns false if storage is full
    bool addWaypoint(int32_t x, int32_t y) {
        if (_waypointCount >= MAX_PERIMETER_WAYPOINTS) {
            DEBUG_PRINTLN("Error: Perimeter storage full!");
            return false;
        }

        if (_waypointCount == 0) {
            // First waypoint - store as origin
            _origin.x = x;
            _origin.y = y;
            _waypointCount = 1;
            _boundsValid = false;
            return true;
        }

        // Calculate relative offset from last waypoint
        Point2D_int lastPoint = getWaypoint(_waypointCount - 1);
        int32_t dx = x - lastPoint.x;
        int32_t dy = y - lastPoint.y;

        // Check if offset fits in 16-bit range (±32767mm = ±32.7m)
        if (dx < -32767 || dx > 32767 || dy < -32767 || dy > 32767) {
            DEBUG_PRINT("Error: Waypoint offset too large: dx=");
            DEBUG_PRINT(dx);
            DEBUG_PRINT(" dy=");
            DEBUG_PRINTLN(dy);
            DEBUG_PRINTLN("Max segment length is 32.7m");
            return false;
        }

        // Store relative offset
        _waypoints[_waypointCount - 1].dx = (int16_t)dx;
        _waypoints[_waypointCount - 1].dy = (int16_t)dy;
        _waypointCount++;
        _boundsValid = false;

        return true;
    }

    // Add waypoint from Point2D_int
    bool addWaypoint(const Point2D_int& point) {
        return addWaypoint(point.x, point.y);
    }

    // Get waypoint at index (returns absolute coordinates)
    Point2D_int getWaypoint(int index) const {
        if (index < 0 || index >= _waypointCount) {
            DEBUG_PRINT("Error: Invalid waypoint index ");
            DEBUG_PRINTLN(index);
            return Point2D_int{0, 0};
        }

        if (index == 0) {
            return _origin;
        }

        // Reconstruct absolute position by summing relative offsets
        Point2D_int pos = _origin;
        for (int i = 0; i < index; i++) {
            pos.x += _waypoints[i].dx;
            pos.y += _waypoints[i].dy;
        }

        return pos;
    }

    // Get all waypoints (reconstructs absolute coordinates)
    // Returns number of waypoints copied
    int getWaypoints(Point2D_int* buffer, int maxCount) const {
        int count = (_waypointCount < maxCount) ? _waypointCount : maxCount;

        Point2D_int pos = _origin;
        buffer[0] = pos;

        for (int i = 1; i < count; i++) {
            pos.x += _waypoints[i - 1].dx;
            pos.y += _waypoints[i - 1].dy;
            buffer[i] = pos;
        }

        return count;
    }

    // Get waypoint count
    int getCount() const {
        return _waypointCount;
    }

    // Clear all waypoints
    void clear() {
        _waypointCount = 0;
        _origin.x = 0;
        _origin.y = 0;
        _boundsValid = false;
    }

    // Calculate bounding box
    void calculateBounds() {
        if (_waypointCount == 0) {
            _minX = _maxX = _minY = _maxY = 0;
            _boundsValid = true;
            return;
        }

        _minX = _maxX = _origin.x;
        _minY = _maxY = _origin.y;

        Point2D_int pos = _origin;
        for (int i = 0; i < _waypointCount - 1; i++) {
            pos.x += _waypoints[i].dx;
            pos.y += _waypoints[i].dy;

            if (pos.x < _minX) _minX = pos.x;
            if (pos.x > _maxX) _maxX = pos.x;
            if (pos.y < _minY) _minY = pos.y;
            if (pos.y > _maxY) _maxY = pos.y;
        }

        _boundsValid = true;

        DEBUG_PRINT("Bounds: (");
        DEBUG_PRINT(_minX);
        DEBUG_PRINT(",");
        DEBUG_PRINT(_minY);
        DEBUG_PRINT(") to (");
        DEBUG_PRINT(_maxX);
        DEBUG_PRINT(",");
        DEBUG_PRINT(_maxY);
        DEBUG_PRINTLN(")");
    }

    // Get bounding box
    void getBounds(int32_t& minX, int32_t& maxX, int32_t& minY, int32_t& maxY) {
        if (!_boundsValid) {
            calculateBounds();
        }
        minX = _minX;
        maxX = _maxX;
        minY = _minY;
        maxY = _maxY;
    }

    // Get bounding box width in mm
    int32_t getWidth() {
        if (!_boundsValid) calculateBounds();
        return _maxX - _minX;
    }

    // Get bounding box height in mm
    int32_t getHeight() {
        if (!_boundsValid) calculateBounds();
        return _maxY - _minY;
    }

    // Load from array of absolute coordinates
    bool loadFromArray(const Point2D_int* points, int count) {
        clear();

        if (count > MAX_PERIMETER_WAYPOINTS) {
            DEBUG_PRINT("Error: Too many waypoints (");
            DEBUG_PRINT(count);
            DEBUG_PRINT("), max is ");
            DEBUG_PRINTLN(MAX_PERIMETER_WAYPOINTS);
            return false;
        }

        for (int i = 0; i < count; i++) {
            if (!addWaypoint(points[i])) {
                return false;
            }
        }

        return true;
    }

    // Calculate perimeter length (total distance)
    int32_t calculatePerimeterLength() const {
        if (_waypointCount < 2) return 0;

        int32_t totalLength = 0;

        // Sum segment lengths
        for (int i = 0; i < _waypointCount - 1; i++) {
            int32_t dx = _waypoints[i].dx;
            int32_t dy = _waypoints[i].dy;
            totalLength += GeometryUtils::vectorLength(dx, dy);
        }

        // Add closing segment (last to first)
        Point2D_int last = getWaypoint(_waypointCount - 1);
        totalLength += GeometryUtils::distanceBetweenPoints(last, _origin);

        return totalLength;
    }

    // Get memory usage in bytes
    int getMemoryUsage() const {
        // Origin (8 bytes) + waypoints (4 bytes each) + overhead
        return sizeof(Point2D_int) + (_waypointCount - 1) * sizeof(RelativeWaypoint);
    }

    // Print statistics
    void printStats() const {
        DEBUG_PRINT("Perimeter: ");
        DEBUG_PRINT(_waypointCount);
        DEBUG_PRINT(" waypoints, ");
        DEBUG_PRINT(getMemoryUsage());
        DEBUG_PRINT(" bytes (");
        DEBUG_PRINT((getMemoryUsage() * 100) / (MAX_PERIMETER_WAYPOINTS * 4));
        DEBUG_PRINTLN("% of max)");

        if (_waypointCount > 0) {
            DEBUG_PRINT("Area: ");
            DEBUG_PRINT(getWidth());
            DEBUG_PRINT("mm × ");
            DEBUG_PRINT(getHeight());
            DEBUG_PRINTLN("mm");
        }
    }

    // Check if a point is approximately on the perimeter (within threshold)
    bool isOnPerimeter(const Point2D_int& point, distance_t threshold_mm = 500) const {
        if (_waypointCount < 2) return false;

        Point2D_int prev = _origin;

        // Check each segment
        for (int i = 1; i < _waypointCount; i++) {
            Point2D_int curr = getWaypoint(i);

            int32_t dist = GeometryUtils::distanceToLineSegment(point, prev, curr);

            if (dist <= threshold_mm) {
                return true;
            }

            prev = curr;
        }

        // Check closing segment
        int32_t dist = GeometryUtils::distanceToLineSegment(point, prev, _origin);
        return dist <= threshold_mm;
    }
};

#endif
