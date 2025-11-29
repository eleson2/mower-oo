#ifndef LINE_FOLLOWER_H
#define LINE_FOLLOWER_H

#include "globals.hpp"
#include "GPSInterface.h"
#include "IMUInterface.h"
#include "IntegerMathDefault.h"
#include "DriveUnit.h"
#include <TaskSchedulerDeclarations.h>

// Line Following Controller - INTEGER ONLY VERSION
// Smoothly guides mower along a line from point A to point B
// All math in integers: distances in mm, angles in tenths of degrees
class LineFollower : public Task {
private:
    // Line definition (in millimeters)
    Point2D_int _startPoint;
    Point2D_int _endPoint;
    bool _lineSet;

    // Sensor references
    GPSInterface* _gps;
    IMUInterface* _imu;
    DriveUnit* _drive;

    // Current state
    Point2D_int _currentPosition;    // mm
    angle_t _currentHeading;         // tenths of degrees (0-3599)

    // Controller parameters (tunable)
    int16_t _K_crossTrack;           // Cross-track gain (scaled by 1000)
    int16_t _K_heading;              // Heading gain (scaled by 1000)
    distance_t _lookaheadDistance;   // Look-ahead distance in mm
    wheelSpeed _baseSpeed;           // Forward speed

    // Completion detection
    distance_t _completionThreshold; // Distance to endpoint in mm

    bool _lineComplete;

    // Helper functions
    distance_t calculateCrossTrackError();
    int16_t calculateHeadingError();
    Point2D_int calculateNearestPointOnLine();
    Point2D_int calculateLookAheadPoint();
    distance_t calculateDistanceToEnd();
    angle_t calculateBearing(const Point2D_int& from, const Point2D_int& to);

public:
    // Constructor
    LineFollower(Scheduler* aS, GPSInterface* gps, IMUInterface* imu, DriveUnit* drive);
    ~LineFollower();

    // Set the line to follow (coordinates in millimeters)
    void setLineMM(Point2D_int start, Point2D_int end);

    // Set line using Point2D_int (convenience wrapper)
    void setLine(Point2D_int start, Point2D_int end) { setLineMM(start, end); }

    // Set line in meters (for convenience)
    void setLineMeters(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

    // Set controller parameters
    // Gains are scaled by 1000 (e.g., 1000 = 1.0, 1500 = 1.5)
    void setCrossTrackGain(int16_t gain) { _K_crossTrack = gain; }
    void setHeadingGain(int16_t gain) { _K_heading = gain; }
    void setLookaheadDistanceMM(distance_t distance) { _lookaheadDistance = distance; }
    void setLookaheadDistanceMeters(int meters) { _lookaheadDistance = METERS_TO_MM(meters); }
    void setBaseSpeed(wheelSpeed speed) { _baseSpeed = speed; }
    void setCompletionThresholdMM(distance_t threshold) { _completionThreshold = threshold; }

    // Get controller parameters
    int16_t getCrossTrackGain() const { return _K_crossTrack; }
    int16_t getHeadingGain() const { return _K_heading; }
    distance_t getLookaheadDistance() const { return _lookaheadDistance; }
    wheelSpeed getBaseSpeed() const { return _baseSpeed; }

    // Update position and heading from sensors
    void updateSensors();

    // Check if line following is complete
    bool isComplete() const { return _lineComplete; }

    // Get current cross-track error in mm (for monitoring)
    distance_t getCrossTrackError() { return calculateCrossTrackError(); }

    // Reset state
    void reset();

    // Task callbacks
    bool Callback() override;
    bool OnEnable() override;
    void OnDisable() override;
};

#endif
