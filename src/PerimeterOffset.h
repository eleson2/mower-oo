#ifndef PERIMETEROFFSET_H
#define PERIMETEROFFSET_H

#include "globals.hpp"
#include "Arduino.h"
#include "PerimeterStorage.h"
#include "IntegerMath.h"
#include "GeometryUtils.h"

// Maximum waypoints in offset perimeter
#define MAX_OFFSET_WAYPOINTS 1000

// Generates inward offset perimeters from original perimeter
// Used for multi-lap perimeter following and buffer zone creation
// All math is INTEGER ONLY - no floating point!
class PerimeterOffset {
private:
    PerimeterStorage* _originalPerimeter;

    // Offset perimeter storage
    Point2D_int _offsetWaypoints[MAX_OFFSET_WAYPOINTS];
    int _offsetCount;

    // Current offset distance (negative = inward)
    int _currentOffset_mm;

public:
    PerimeterOffset(PerimeterStorage* originalPerimeter)
        : _originalPerimeter(originalPerimeter),
          _offsetCount(0),
          _currentOffset_mm(0) {
    }

    // Generate inward offset perimeter
    // offset_mm: distance to move inward (positive value, e.g., 250 for 0.25m inward)
    // Returns: number of waypoints in offset perimeter (0 on error)
    int generateInwardOffset(int offset_mm) {
        if (!_originalPerimeter || _originalPerimeter->getCount() < 3) {
            DEBUG_PRINTLN("Error: Invalid original perimeter");
            return 0;
        }

        if (offset_mm < 0) {
            DEBUG_PRINTLN("Error: Offset must be positive for inward");
            return 0;
        }

        _currentOffset_mm = offset_mm;
        _offsetCount = 0;

        int originalCount = _originalPerimeter->getCount();

        DEBUG_PRINT("Generating inward offset: ");
        DEBUG_PRINT(offset_mm);
        DEBUG_PRINTLN("mm");

        // Process each segment of the perimeter
        for (int i = 0; i < originalCount; i++) {
            Point2D_int curr = _originalPerimeter->getWaypoint(i);
            Point2D_int next = _originalPerimeter->getWaypoint((i + 1) % originalCount);
            Point2D_int prev = _originalPerimeter->getWaypoint((i - 1 + originalCount) % originalCount);

            // Calculate offset point for current vertex
            Point2D_int offsetPoint = calculateVertexOffset(prev, curr, next, offset_mm);

            // Add to offset perimeter
            if (_offsetCount < MAX_OFFSET_WAYPOINTS) {
                _offsetWaypoints[_offsetCount] = offsetPoint;
                _offsetCount++;
            } else {
                DEBUG_PRINTLN("Error: Offset waypoint buffer full!");
                return 0;
            }
        }

        DEBUG_PRINT("Generated ");
        DEBUG_PRINT(_offsetCount);
        DEBUG_PRINTLN(" offset waypoints");

        return _offsetCount;
    }

    // Get offset waypoint at index
    Point2D_int getOffsetWaypoint(int index) const {
        if (index < 0 || index >= _offsetCount) {
            return Point2D_int{0, 0};
        }
        return _offsetWaypoints[index];
    }

    // Get all offset waypoints
    int getOffsetWaypoints(Point2D_int* buffer, int maxCount) const {
        int count = (_offsetCount < maxCount) ? _offsetCount : maxCount;
        for (int i = 0; i < count; i++) {
            buffer[i] = _offsetWaypoints[i];
        }
        return count;
    }

    // Get offset waypoint count
    int getOffsetCount() const {
        return _offsetCount;
    }

    // Get current offset distance
    int getCurrentOffset() const {
        return _currentOffset_mm;
    }

private:
    // Calculate offset point for a vertex given its neighbors
    // Uses angle bisector method for smooth corners
    Point2D_int calculateVertexOffset(const Point2D_int& prev,
                                       const Point2D_int& curr,
                                       const Point2D_int& next,
                                       int offset_mm) {
        // Vector from curr to prev
        int32_t v1x = prev.x - curr.x;
        int32_t v1y = prev.y - curr.y;

        // Vector from curr to next
        int32_t v2x = next.x - curr.x;
        int32_t v2y = next.y - curr.y;

        // Get perpendiculars pointing inward (right-hand for CCW polygons)
        int32_t perp1x, perp1y;
        int32_t perp2x, perp2y;

        GeometryUtils::getPerpendicular(v1x, v1y, true, perp1x, perp1y);
        GeometryUtils::getPerpendicular(v2x, v2y, false, perp2x, perp2y);

        // Normalize perpendiculars (scaled by 1000)
        int32_t norm1x, norm1y;
        int32_t norm2x, norm2y;

        GeometryUtils::normalizeVector(perp1x, perp1y, norm1x, norm1y);
        GeometryUtils::normalizeVector(perp2x, perp2y, norm2x, norm2y);

        // Bisector = average of two perpendiculars
        int32_t bisectorX = (norm1x + norm2x) / 2;
        int32_t bisectorY = (norm1y + norm2y) / 2;

        // Normalize bisector
        int32_t normBisectorX, normBisectorY;
        GeometryUtils::normalizeVector(bisectorX, bisectorY, normBisectorX, normBisectorY);

        // Calculate angle between edges to determine offset scaling
        // For sharp corners, we need to extend the offset
        // offset_scaled = offset / cos(angle/2)

        // Dot product of normalized perpendiculars
        int32_t dot = GeometryUtils::dotProduct(norm1x, norm1y, norm2x, norm2y);

        // cos(angle/2) ≈ sqrt((1 + cos(angle))/2)
        // For simplicity, use a scaling factor based on dot product
        // If perpendiculars are parallel (dot = 1000), scale = 1
        // If perpendiculars are opposite (dot = -1000), corner is very sharp

        int32_t scale = 1000;
        if (dot > 0) {
            // cos(angle/2) ≈ sqrt((1000 + dotProduct)/2000) * 1000
            int32_t cosHalfAngle = GeometryUtils::integerSqrt(((1000 + dot) * 1000) / 2);
            if (cosHalfAngle > 100) {  // Avoid division by very small numbers
                scale = (1000 * 1000) / cosHalfAngle;
            } else {
                scale = 10000;  // Very sharp corner, limit scaling
            }
        } else {
            // Obtuse corner, use larger scale
            scale = 2000;  // 2x offset for obtuse corners
        }

        // Limit scale to reasonable range (1x to 5x)
        if (scale > 5000) scale = 5000;
        if (scale < 1000) scale = 1000;

        // Calculate offset point
        // offset_point = curr + bisector * offset * scale
        int32_t offsetX = curr.x + (normBisectorX * offset_mm * scale) / (1000 * 1000);
        int32_t offsetY = curr.y + (normBisectorY * offset_mm * scale) / (1000 * 1000);

        return Point2D_int{offsetX, offsetY};
    }
};

#endif
