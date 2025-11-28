#include "LineFollower.h"

// Constructor
LineFollower::LineFollower(Scheduler* aS, GPSInterface* gps, IMUInterface* imu, DriveUnit* drive)
    : Task(200, TASK_FOREVER, aS, false),  // 200ms update rate, disabled initially
      _lineSet(false),
      _gps(gps),
      _imu(imu),
      _drive(drive),
      _K_crossTrack(1000),           // Default: 1.0 (scaled by 1000)
      _K_heading(2000),              // Default: 2.0 (scaled by 1000)
      _lookaheadDistance(1000),      // Default: 1000mm = 1 meter
      _baseSpeed(Speed50),           // Default: 50% speed
      _completionThreshold(300),     // Default: 300mm = 30cm
      _lineComplete(false)
{
}

// Destructor
LineFollower::~LineFollower() {
}

// Set the line to follow (in millimeters)
void LineFollower::setLineMM(Point2D_int start, Point2D_int end) {
    _startPoint = start;
    _endPoint = end;
    _lineSet = true;
    _lineComplete = false;
}

// Set line in meters
void LineFollower::setLineMeters(int32_t x1, int32_t y1, int32_t x2, int32_t y2) {
    setLineMM(
        Point2D_int(METERS_TO_MM(x1), METERS_TO_MM(y1)),
        Point2D_int(METERS_TO_MM(x2), METERS_TO_MM(y2))
    );
}

// Reset state
void LineFollower::reset() {
    _lineComplete = false;
}

// Update sensors (read GPS and IMU)
void LineFollower::updateSensors() {
    if (_gps && _gps->hasFix()) {
        _currentPosition = _gps->getPosition();
    }

    if (_imu && _imu->isInitialized()) {
        _currentHeading = _imu->getHeading();
    }
}

// Calculate bearing from one point to another (returns tenths of degrees)
angle_t LineFollower::calculateBearing(const Point2D_int& from, const Point2D_int& to) {
    distance_t dx = to.x - from.x;
    distance_t dy = to.y - from.y;

    // Use integer atan2
    angle_t bearing = atan2_int(dy, dx);

    // atan2 gives mathematical angle (0° = East, 90° = North)
    // Convert to compass bearing (0° = North, 90° = East)
    bearing = 900 - bearing;  // Rotate 90° clockwise
    return normalizeAngle(bearing);
}

// Calculate nearest point on line segment
Point2D_int LineFollower::calculateNearestPointOnLine() {
    // Vector from start to end of line
    Point2D_int lineVector = _endPoint - _startPoint;

    // Vector from start to current position
    Point2D_int posVector = _currentPosition - _startPoint;

    // Project position onto line (parametric t)
    int64_t lineLengthSquared = lineVector.dot(lineVector);

    if (lineLengthSquared < 100) {  // Line too short (< 10mm)
        return _startPoint;
    }

    // t = (posVector · lineVector) / |lineVector|²
    // Scale t by 1000 for precision
    int32_t t_scaled = (int32_t)((posVector.dot(lineVector) * 1000) / lineLengthSquared);

    // Clamp to line segment [0, 1000]
    if (t_scaled < 0) t_scaled = 0;
    if (t_scaled > 1000) t_scaled = 1000;

    // Calculate nearest point
    Point2D_int nearestPoint(
        _startPoint.x + (lineVector.x * t_scaled / 1000),
        _startPoint.y + (lineVector.y * t_scaled / 1000)
    );

    return nearestPoint;
}

// Calculate cross-track error (perpendicular distance from line in mm)
distance_t LineFollower::calculateCrossTrackError() {
    if (!_lineSet) return 0;

    // Vector from start to end
    Point2D_int lineVector = _endPoint - _startPoint;

    // Vector from start to current position
    Point2D_int posVector = _currentPosition - _startPoint;

    // Cross-track error = (posVector × lineVector) / |lineVector|
    int64_t crossProduct = posVector.cross(lineVector);
    distance_t lineMagnitude = lineVector.magnitude();

    if (lineMagnitude < 10) return 0;  // Avoid division by near-zero

    // Return signed distance in mm
    return (distance_t)(crossProduct / lineMagnitude);
}

// Calculate look-ahead point on the line
Point2D_int LineFollower::calculateLookAheadPoint() {
    // Get nearest point on line
    Point2D_int nearestPoint = calculateNearestPointOnLine();

    // Direction vector along line (normalized to magnitude 1000)
    Point2D_int lineDirection = (_endPoint - _startPoint).normalized();

    // Move ahead by lookahead distance
    // lookahead point = nearest + direction * distance / 1000
    Point2D_int lookAheadPoint(
        nearestPoint.x + (lineDirection.x * _lookaheadDistance / 1000),
        nearestPoint.y + (lineDirection.y * _lookaheadDistance / 1000)
    );

    // Don't go past the end point
    distance_t distToEnd = lookAheadPoint.distanceTo(_endPoint);
    distance_t nearestToEnd = nearestPoint.distanceTo(_endPoint);

    if (distToEnd > nearestToEnd) {
        // Lookahead would overshoot, aim for endpoint instead
        lookAheadPoint = _endPoint;
    }

    return lookAheadPoint;
}

// Calculate heading error (tenths of degrees, signed)
int16_t LineFollower::calculateHeadingError() {
    if (!_lineSet) return 0;

    // Calculate look-ahead point
    Point2D_int lookAheadPoint = calculateLookAheadPoint();

    // Desired heading is bearing to look-ahead point
    angle_t desiredHeading = calculateBearing(_currentPosition, lookAheadPoint);

    // Heading error (shortest angular distance)
    int16_t headingError = angleDifference(desiredHeading, _currentHeading);

    return headingError;
}

// Calculate distance to end point (mm)
distance_t LineFollower::calculateDistanceToEnd() {
    return _currentPosition.distanceTo(_endPoint);
}

// Task enable callback
bool LineFollower::OnEnable() {
    if (!_lineSet) {
        return false;  // Can't enable without a line set
    }

    _lineComplete = false;
    updateSensors();

    return true;
}

// Task disable callback
void LineFollower::OnDisable() {
    // Stop the drive unit when disabled
    if (_drive) {
        _drive->setTargetSpeed(0, 0, 200);
    }
}

// Main control loop callback - ALL INTEGER MATH!
bool LineFollower::Callback() {
    if (!_lineSet) {
        return false;  // Stop task
    }

    // Update sensor readings
    updateSensors();

    // Check if we've reached the end
    distance_t distanceToEnd = calculateDistanceToEnd();
    if (distanceToEnd < _completionThreshold) {
        _lineComplete = true;

        // Stop motors
        if (_drive) {
            _drive->setTargetSpeed(0, 0, 200);
        }

        return false;  // Stop task - line complete
    }

    // Calculate errors
    distance_t crossTrackError = calculateCrossTrackError();  // mm
    int16_t headingError = calculateHeadingError();           // tenths of degrees

    // Calculate steering correction (integer math only!)
    // correction = (K_cte * CTE) + (K_heading * HE)
    // K values are scaled by 1000, so divide result by 1000

    // Cross-track contribution (K scaled by 1000, CTE in mm)
    int32_t cteContribution = ((int32_t)_K_crossTrack * crossTrackError) / 1000;

    // Heading contribution (K scaled by 1000, HE in tenths of degrees)
    // Scale heading error to be comparable to distance
    // Heading error of 100 tenths (10°) should produce similar effect as 100mm CTE
    int32_t headingContribution = ((int32_t)_K_heading * headingError) / 1000;

    // Total steering correction
    int32_t steeringCorrection = cteContribution + headingContribution;

    // Limit correction to ±50% of max speed
    int32_t maxCorrection = MaxSpeed / 2;
    if (steeringCorrection > maxCorrection) steeringCorrection = maxCorrection;
    if (steeringCorrection < -maxCorrection) steeringCorrection = -maxCorrection;

    // Apply to differential drive
    wheelSpeed leftSpeed = _baseSpeed - (wheelSpeed)steeringCorrection;
    wheelSpeed rightSpeed = _baseSpeed + (wheelSpeed)steeringCorrection;

    // Clamp to valid range
    if (leftSpeed > MaxSpeed) leftSpeed = MaxSpeed;
    if (leftSpeed < -MaxSpeed) leftSpeed = -MaxSpeed;
    if (rightSpeed > MaxSpeed) rightSpeed = MaxSpeed;
    if (rightSpeed < -MaxSpeed) rightSpeed = -MaxSpeed;

    // Send to drive unit
    if (_drive) {
        _drive->setTargetSpeed(leftSpeed, rightSpeed, getInterval());
    }

    return true;  // Continue task
}
