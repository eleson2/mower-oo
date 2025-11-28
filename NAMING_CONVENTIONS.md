# Naming Conventions - Mower Control Codebase

## Current State Analysis

### Issues Identified

1. **File name inconsistencies**: `driveunit.h` vs `DriveUnit_new.h`, `sensorSonar.cpp` vs `SensorSonar.h`
2. **Include guard inconsistencies**: Mix of `_UPPERCASE_H`, `_lowercase_h`, `_MixedCase_h`
3. **Class name inconsistencies**: `allMovements` (camelCase) vs `LineFollower` (PascalCase)
4. **Member variable inconsistencies**: Some use `_prefix`, some use `PascalCase`, some use `camelCase`
5. **Function name inconsistencies**: Mix of `PascalCase` and `camelCase`

---

## Recommended C++ Naming Standards

### 1. Files

**Standard**: PascalCase matching the primary class name

```
✅ LineFollower.h / LineFollower.cpp
✅ DriveUnit.h / DriveUnit.cpp
✅ GPSInterface.h
✅ IMUInterface.h
✅ IntegerMath.h

❌ driveunit.h (should be DriveUnit.h)
❌ sensorSonar.cpp (should be SensorSonar.cpp)
❌ motor.hpp (should be Motor.hpp)
❌ globals.hpp (should be Globals.hpp)
❌ queue.h (should be Queue.h)
```

**Exception**: Files with multiple utilities can use descriptive names
```
✅ Globals.hpp
✅ IntegerMath.h
```

---

### 2. Include Guards

**Standard**: `FILENAME_H` (no leading underscores - reserved by C++ standard)

```cpp
✅ #ifndef LINE_FOLLOWER_H
   #define LINE_FOLLOWER_H

❌ #ifndef _LINE_FOLLOWER_H  // Leading underscore reserved
❌ #ifndef _Wheel_h          // Inconsistent case
❌ #ifndef _motor_h          // Lowercase
```

---

### 3. Classes and Structs

**Standard**: PascalCase

```cpp
✅ class LineFollower
✅ class DriveUnit
✅ class GPSInterface
✅ struct Point2D_int  // Exception: descriptive suffix

❌ class allMovements  // Should be AllMovements
❌ class sSonar        // Should be SonarSensor or SensorSonar
```

---

### 4. Member Variables

**Standard**: camelCase with leading underscore `_`

```cpp
class LineFollower {
private:
    ✅ Point2D_int _startPoint;
    ✅ Point2D_int _endPoint;
    ✅ GPSInterface* _gps;
    ✅ IMUInterface* _imu;
    ✅ int16_t _K_crossTrack;  // Exception: preserve semantic meaning

    ❌ int TargetSpeed;  // Should be _targetSpeed
    ❌ int CurSpeed;     // Should be _currentSpeed
};
```

**Rationale**: Leading underscore clearly distinguishes member variables from local variables and parameters

---

### 5. Member Functions (Methods)

**Standard**: camelCase (verb-first for actions)

```cpp
✅ void setLineMM(...)
✅ void updateSensors()
✅ distance_t calculateCrossTrackError()
✅ bool isComplete() const
✅ void reset()

❌ void EmitNewSpeed()    // Old style - should be emitNewSpeed()
❌ void EmitTargetSpeed() // Old style - should be emitTargetSpeed()
```

**Exception**: Override methods from external libraries keep their style
```cpp
✅ bool Callback() override;    // TaskScheduler convention
✅ bool OnEnable() override;    // TaskScheduler convention
✅ void OnDisable() override;   // TaskScheduler convention
```

---

### 6. Function Parameters

**Standard**: camelCase (no prefix)

```cpp
✅ void setTargetSpeed(int leftSpeed, int rightSpeed, int timeToReach)
✅ LineFollower(Scheduler* scheduler, GPSInterface* gps, IMUInterface* imu)

❌ void setTargetSpeed(int _leftspeed, int _rightspeed)  // Unnecessary prefix
```

---

### 7. Local Variables

**Standard**: camelCase (no prefix)

```cpp
✅ distance_t crossTrackError = calculateCrossTrackError();
✅ Point2D_int nearestPoint = calculateNearestPointOnLine();
✅ int32_t steeringCorrection = ...;

❌ int Speed = ...;  // Should be speed or targetSpeed
```

---

### 8. Constants

**Standard**:
- Compile-time constants: PascalCase or SCREAMING_SNAKE_CASE
- Runtime constants: camelCase with `k` prefix

```cpp
✅ constexpr wheelSpeed MaxSpeed = 1000;
✅ constexpr angle_t ANGLE_90 = 900;
✅ constexpr wheelSpeed Speed50 = 500;

✅ const int kDefaultTimeout = 1000;  // Alternative style
```

---

### 9. Macros and Preprocessor

**Standard**: SCREAMING_SNAKE_CASE

```cpp
✅ #define DEBUG_ENABLED 1
✅ #define DEGREES_TO_ANGLE(deg) ((angle_t)((deg) * 10))
✅ #define METERS_TO_MM(m) ((distance_t)((m) * 1000))

❌ #define _TASK_OO_CALLBACKS  // Leading underscore reserved
```

---

### 10. Type Aliases (typedef)

**Standard**: snake_case with `_t` suffix (C style) or PascalCase (C++ style)

```cpp
✅ typedef int16_t angle_t;
✅ typedef int32_t distance_t;
✅ typedef uint32_t time_ms_t;
✅ typedef int16_t wheelSpeed;  // Also acceptable

// For complex types, PascalCase is acceptable
✅ typedef void (*MotorSpeedCallback)(movement m);
```

---

### 11. Enums

**Standard**:
- Enum type: PascalCase
- Enum values: SCREAMING_SNAKE_CASE

```cpp
✅ enum MotionState {
    MOTION_IDLE,
    MOTION_PATTERN,
    MOTION_LINE_FOLLOW,
    MOTION_OBSTACLE_AVOID
};

// Alternative C++11 style (scoped enum)
✅ enum class MotionState {
    Idle,         // PascalCase when scoped
    Pattern,
    LineFollow,
    ObstacleAvoid
};
```

**Current code** (needs update):
```cpp
❌ typedef enum {
    CONTINOUS,      // Typo: should be CONTINUOUS
    CHARGER_BACKOUT,
    BWFLEFT,        // Should be BWF_LEFT
    BWFRIGHT        // Should be BWF_RIGHT
} CurrentMotion;
```

---

## Priority Fixes

### High Priority (Core Interfaces)

1. **Rename `driveunit.h` → `DriveUnit.h`** and update all includes
2. **Rename `sensorSonar.cpp` → `SensorSonar.cpp`**
3. **Fix `allMovements` → `AllMovements`** (class name)
4. **Fix `sSonar` → `SonarSensor`** (class name)
5. **Fix include guards** to remove leading underscores

### Medium Priority (Consistency)

6. **Standardize member variables** in `Wheel.h`:
   - `TargetSpeed` → `_targetSpeed`
   - `CurSpeed` → `_currentSpeed`
   - `SpeedIncrement` → `_speedIncrement`

7. **Standardize function names** in older classes:
   - `EmitNewSpeed()` → `emitNewSpeed()`
   - `EmitTargetSpeed()` → `emitTargetSpeed()`

8. **Fix enum values**:
   - `CONTINOUS` → `CONTINUOUS`
   - `BWFLEFT` → `BWF_LEFT`
   - `BWFRIGHT` → `BWF_RIGHT`

### Low Priority (Polish)

9. **Rename utility files**:
   - `motor.hpp` → `Motor.hpp`
   - `globals.hpp` → `Globals.hpp`
   - `queue.h` → `Queue.h`

10. **Update parameter names** to remove underscores:
    - `void setTargetSpeed(int _leftspeed, ...)` → `void setTargetSpeed(int leftSpeed, ...)`

---

## Examples: Before and After

### File: Wheel.h

**Before:**
```cpp
#ifndef _Wheel_h
#define _Wheel_h

class Wheel : L298 {
private:
   int  TargetSpeed;
   int  CurSpeed;
   int  SpeedIncrement;

public:
   void EmitNewSpeed() {
      CurSpeed += SpeedIncrement;
      move(CurSpeed);
   };
};
#endif
```

**After:**
```cpp
#ifndef WHEEL_H
#define WHEEL_H

class Wheel : L298 {
private:
   int _targetSpeed;
   int _currentSpeed;
   int _speedIncrement;

public:
   void emitNewSpeed() {
      _currentSpeed += _speedIncrement;
      move(_currentSpeed);
   }
};
#endif
```

---

### File: allMoves.h → AllMoves.h

**Before:**
```cpp
#ifndef _ALL_MOVES_H
#define _ALL_MOVES_H

class allMovements : public Task {
   // ...
};
#endif
```

**After:**
```cpp
#ifndef ALL_MOVES_H
#define ALL_MOVES_H

class AllMovements : public Task {
   // ...
};
#endif
```

---

## Summary

**Adopt Google C++ Style Guide principles:**
- Files: PascalCase
- Classes: PascalCase
- Functions: camelCase
- Members: _camelCase (with underscore prefix)
- Constants: PascalCase or SCREAMING_SNAKE_CASE
- Macros: SCREAMING_SNAKE_CASE
- Include guards: FILENAME_H (no leading underscore)

**Benefits:**
- ✅ Consistent, readable code
- ✅ Clear distinction between members and locals
- ✅ Follows modern C++ conventions
- ✅ Avoids reserved identifiers (leading underscores)
