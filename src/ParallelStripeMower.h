#ifndef PARALLELSTRIPEMOWER_H
#define PARALLELSTRIPEMOWER_H

#include "globals.hpp"
#include "Arduino.h"
#include "GPSInterface.h"
#include "IMUInterface.h"
#include "LineFollower.h"
#include "IntegerMath.h"
#include "PerimeterStorage.h"
#include "PerimeterOffset.h"

// Maximum waypoints for turns
#define MAX_ARC_WAYPOINTS 16

// Parallel stripe mowing pattern with teardrop turns
// Uses existing LineFollower for straight lines and arc segments
class ParallelStripeMower {
private:
    GPSInterface* _gps;
    IMUInterface* _imu;
    LineFollower* _lineFollower;

    // Mowing parameters
    int _stripeWidth_mm;           // Blade width (default 250mm)
    int _turnRadius_mm;            // Turn radius (default 500mm)
    int _bufferZone_mm;            // Perimeter buffer (default 750mm = 3 laps)
    int _perimeterLaps;            // Number of perimeter laps (default 3)

    // Perimeter storage (efficient relative coordinates)
    PerimeterStorage _perimeter;

    // Perimeter offset generator
    PerimeterOffset _perimeterOffset;

    // Stripe tracking
    int _currentStripe;
    bool _movingRight;             // Direction of current stripe
    int _totalStripes;

    // Mowing area bounds (calculated from perimeter)
    int32_t _minX, _maxX;          // Bounding box in mm
    int32_t _minY, _maxY;

    // State machine
    enum MowingState {
        IDLE,
        PERIMETER_LAPS,
        MOWING_STRIPE,
        EXECUTING_TURN,
        COMPLETE
    };
    MowingState _state;
    int _currentLap;

public:
    ParallelStripeMower(GPSInterface* gps, IMUInterface* imu, LineFollower* lineFollower)
        : _gps(gps), _imu(imu), _lineFollower(lineFollower),
          _stripeWidth_mm(250), _turnRadius_mm(500), _bufferZone_mm(750),
          _perimeterLaps(3),
          _perimeterOffset(&_perimeter),
          _currentStripe(0), _movingRight(true), _totalStripes(0),
          _minX(0), _maxX(0), _minY(0), _maxY(0),
          _state(IDLE), _currentLap(0) {
    }

    // Set mowing blade width (in mm)
    void setStripeWidth(int width_mm) {
        _stripeWidth_mm = width_mm;
    }

    // Set turn radius (in mm) - larger = gentler turns
    void setTurnRadius(int radius_mm) {
        _turnRadius_mm = radius_mm;
    }

    // Set buffer zone width (in mm)
    void setBufferZone(int buffer_mm) {
        _bufferZone_mm = buffer_mm;
    }

    // Set number of perimeter laps
    void setPerimeterLaps(int laps) {
        _perimeterLaps = laps;
    }

    // Define perimeter from GPS waypoints
    void setPerimeter(const Point2D_int* waypoints, int count) {
        if (!_perimeter.loadFromArray(waypoints, count)) {
            DEBUG_PRINTLN("Error: Failed to load perimeter");
            return;
        }

        calculateBoundingBox();
        calculateTotalStripes();

        _perimeter.printStats();
        DEBUG_PRINT("Calculated ");
        DEBUG_PRINT(_totalStripes);
        DEBUG_PRINTLN(" stripes");
    }

    // Get perimeter storage (for direct access)
    PerimeterStorage* getPerimeterStorage() {
        return &_perimeter;
    }

    // Start mowing pattern
    void startMowing() {
        if (_perimeter.getCount() < 3) {
            DEBUG_PRINTLN("Error: Need at least 3 perimeter points");
            return;
        }

        _state = PERIMETER_LAPS;
        _currentLap = 0;
        _currentStripe = 0;
        _movingRight = true;

        DEBUG_PRINTLN("Starting mowing pattern");
        DEBUG_PRINT("Perimeter laps: ");
        DEBUG_PRINTLN(_perimeterLaps);

        // Start first perimeter lap
        startPerimeterLap();
    }

    // Update state machine (call frequently from main loop)
    void update() {
        switch (_state) {
            case PERIMETER_LAPS:
                // Check if current lap is complete
                if (_lineFollower->isComplete()) {
                    _currentLap++;

                    if (_currentLap < _perimeterLaps) {
                        // Start next perimeter lap
                        startPerimeterLap();
                    } else {
                        // Perimeter laps done, start mowing stripes
                        DEBUG_PRINTLN("Perimeter laps complete - starting stripes");
                        _state = MOWING_STRIPE;
                        _currentStripe = 0;
                        startNextStripe();
                    }
                }
                break;

            case MOWING_STRIPE:
                // Check if stripe is complete
                if (_lineFollower->isComplete()) {
                    _lineFollower->disable();

                    if (_currentStripe >= _totalStripes - 1) {
                        // All stripes complete!
                        _state = COMPLETE;
                        DEBUG_PRINTLN("Mowing complete!");
                    } else {
                        // Execute turn to next stripe
                        _state = EXECUTING_TURN;
                        startTurn();
                    }
                }
                break;

            case EXECUTING_TURN:
                // Turn execution handled by arc waypoints
                // This is a placeholder - actual implementation would
                // track progress through arc waypoints
                if (_lineFollower->isComplete()) {
                    _currentStripe++;
                    _movingRight = !_movingRight;
                    _state = MOWING_STRIPE;
                    startNextStripe();
                }
                break;

            case COMPLETE:
            case IDLE:
            default:
                // Do nothing
                break;
        }
    }

    // Get current state
    MowingState getState() const {
        return _state;
    }

    // Check if mowing is complete
    bool isComplete() const {
        return _state == COMPLETE;
    }

private:
    // Calculate bounding box of perimeter
    void calculateBoundingBox() {
        _perimeter.getBounds(_minX, _maxX, _minY, _maxY);

        DEBUG_PRINT("Bounding box: (");
        DEBUG_PRINT(_minX);
        DEBUG_PRINT(",");
        DEBUG_PRINT(_minY);
        DEBUG_PRINT(") to (");
        DEBUG_PRINT(_maxX);
        DEBUG_PRINT(",");
        DEBUG_PRINT(_maxY);
        DEBUG_PRINTLN(")");
    }

    // Calculate total number of stripes needed
    void calculateTotalStripes() {
        // Width of mowing area (accounting for buffer zone on both sides)
        int32_t mowingWidth = (_maxX - _minX) - (2 * _bufferZone_mm);

        if (mowingWidth <= 0) {
            _totalStripes = 0;
            return;
        }

        _totalStripes = (mowingWidth / _stripeWidth_mm) + 1;

        DEBUG_PRINT("Mowing width: ");
        DEBUG_PRINT(mowingWidth);
        DEBUG_PRINT("mm, stripes: ");
        DEBUG_PRINTLN(_totalStripes);
    }

    // Start a perimeter lap
    void startPerimeterLap() {
        // Follow perimeter with offset based on lap number
        // Lap 0: at perimeter, Lap 1: 250mm in, Lap 2: 500mm in
        int offset_mm = _currentLap * _stripeWidth_mm;

        DEBUG_PRINT("Starting perimeter lap ");
        DEBUG_PRINT(_currentLap);
        DEBUG_PRINT(" with offset ");
        DEBUG_PRINT(offset_mm);
        DEBUG_PRINTLN("mm");

        if (offset_mm == 0) {
            // First lap - follow original perimeter
            Point2D_int start = _perimeter.getWaypoint(0);
            Point2D_int end = _perimeter.getWaypoint(1);

            _lineFollower->setLine(start, end);
            _lineFollower->enable();
        } else {
            // Generate offset perimeter
            int offsetCount = _perimeterOffset.generateInwardOffset(offset_mm);

            if (offsetCount < 2) {
                DEBUG_PRINTLN("Error: Failed to generate offset perimeter");
                return;
            }

            // Follow first segment of offset perimeter
            Point2D_int start = _perimeterOffset.getOffsetWaypoint(0);
            Point2D_int end = _perimeterOffset.getOffsetWaypoint(1);

            _lineFollower->setLine(start, end);
            _lineFollower->enable();
        }
    }

    // Start next mowing stripe
    void startNextStripe() {
        Point2D_int start, end;

        // Calculate stripe Y position (left-to-right stripes in this example)
        int32_t stripeX = _minX + _bufferZone_mm + (_currentStripe * _stripeWidth_mm);

        if (_movingRight) {
            // Moving from bottom to top
            start.x = stripeX;
            start.y = _minY + _bufferZone_mm;
            end.x = stripeX;
            end.y = _maxY - _bufferZone_mm;
        } else {
            // Moving from top to bottom
            start.x = stripeX;
            start.y = _maxY - _bufferZone_mm;
            end.x = stripeX;
            end.y = _minY + _bufferZone_mm;
        }

        DEBUG_PRINT("Stripe ");
        DEBUG_PRINT(_currentStripe);
        DEBUG_PRINT(": (");
        DEBUG_PRINT(start.x);
        DEBUG_PRINT(",");
        DEBUG_PRINT(start.y);
        DEBUG_PRINT(") -> (");
        DEBUG_PRINT(end.x);
        DEBUG_PRINT(",");
        DEBUG_PRINT(end.y);
        DEBUG_PRINTLN(")");

        _lineFollower->setLine(start, end);
        _lineFollower->enable();
    }

    // Execute teardrop turn between stripes
    void startTurn() {
        DEBUG_PRINT("Executing turn from stripe ");
        DEBUG_PRINT(_currentStripe);
        DEBUG_PRINT(" to ");
        DEBUG_PRINTLN(_currentStripe + 1);

        // Generate arc waypoints for smooth turn
        Point2D_int arcWaypoints[MAX_ARC_WAYPOINTS];
        int waypointCount = generateTurnArc(arcWaypoints);

        // For now, just create a single line to next stripe start
        // Full implementation would follow all arc waypoints
        if (waypointCount >= 2) {
            _lineFollower->setLine(arcWaypoints[0], arcWaypoints[waypointCount - 1]);
            _lineFollower->enable();
        }
    }

    // Generate arc waypoints for teardrop turn
    // Returns number of waypoints generated
    int generateTurnArc(Point2D_int* waypoints) {
        // Current stripe end position
        int32_t currentX = _minX + _bufferZone_mm + (_currentStripe * _stripeWidth_mm);
        int32_t currentY = _movingRight ? (_maxY - _bufferZone_mm) : (_minY + _bufferZone_mm);

        // Next stripe start position
        int32_t nextX = _minX + _bufferZone_mm + ((_currentStripe + 1) * _stripeWidth_mm);
        int32_t nextY = _movingRight ? (_minY + _bufferZone_mm) : (_maxY - _bufferZone_mm);

        // Calculate turn center point
        // For a teardrop turn, the arc goes out into the buffer zone
        Point2D_int center;
        if (_movingRight) {
            // Turning at top, arc goes right
            center.x = currentX + _turnRadius_mm;
            center.y = currentY;
        } else {
            // Turning at bottom, arc goes right
            center.x = currentX + _turnRadius_mm;
            center.y = currentY;
        }

        // Generate 8 waypoints around 180° arc (0° to 180° or 180° to 360°)
        // Using 45° increments (0°, 45°, 90°, 135°, 180°)
        int waypointIndex = 0;

        // Starting angle depends on direction
        angle_t startAngle = _movingRight ? ANGLE_90 : ANGLE_270;  // 90° or 270°

        for (int i = 0; i <= 4; i++) {  // 0° to 180° in 45° steps
            angle_t angle = startAngle + (i * 450);  // 450 = 45.0°
            angle = normalizeAngle(angle);

            // Calculate point on arc using lookup table
            int32_t dx = ((int32_t)_turnRadius_mm * cos_lookup(angle)) / 1000;
            int32_t dy = ((int32_t)_turnRadius_mm * sin_lookup(angle)) / 1000;

            waypoints[waypointIndex].x = center.x + dx;
            waypoints[waypointIndex].y = center.y + dy;

            DEBUG_PRINT("  Arc point ");
            DEBUG_PRINT(waypointIndex);
            DEBUG_PRINT(": (");
            DEBUG_PRINT(waypoints[waypointIndex].x);
            DEBUG_PRINT(",");
            DEBUG_PRINT(waypoints[waypointIndex].y);
            DEBUG_PRINTLN(")");

            waypointIndex++;
        }

        // Add final waypoint at next stripe start
        waypoints[waypointIndex].x = nextX;
        waypoints[waypointIndex].y = nextY;
        waypointIndex++;

        return waypointIndex;
    }
};

#endif
