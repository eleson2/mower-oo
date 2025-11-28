# Refactoring Implementation Guide

## What Was Done

### Phase 1: Quick Critical Fixes ✅

1. **Fixed Speed Increment Bug** ([Wheel.h:39](src/Wheel.h#L39))
   ```cpp
   // BEFORE (WRONG):
   SpeedIncrement = (CurSpeed - TargetSpeed) / iterations;

   // AFTER (CORRECT):
   SpeedIncrement = (TargetSpeed - CurSpeed) / iterations;
   ```
   **Impact**: Speed ramping now works correctly!

2. **Added Debug Macros** ([globals.hpp](src/globals.hpp))
   - `DEBUG_PRINT(x)` - Print debug message
   - `DEBUG_PRINTLN(x)` - Print debug message with newline
   - `DEBUG_PRINT2(x, y)` - Print two values
   - Set `DEBUG_ENABLED 0` to disable all debug output

3. **Replaced Serial.print** with debug macros in:
   - [Wheel.h](src/Wheel.h)
   - [driveunit.cpp](src/driveunit.cpp)
   - [allMoves.h](src/allMoves.h)

---

### Phase 2: Structural Improvements ✅

4. **New Wheel Class** ([Wheel_new.h](src/Wheel_new.h))
   - Uses **composition** instead of inheritance
   - Takes `Motor*` in constructor (dependency injection)
   - Clearer conceptual model: Wheel HAS-A motor (not IS-A motor)

5. **New DriveUnit** ([DriveUnit_new.h](src/DriveUnit_new.h))
   - Works with new Wheel class
   - Creates motor drivers internally
   - Cleaner initialization

6. **MotionController Interface** ([MotionController.h](src/MotionController.h))
   - Abstract base class for all motion controllers
   - Common methods: start(), stop(), isActive(), update()
   - Supports state machine integration

7. **PatternController** ([PatternController.h](src/PatternController.h))
   - Wraps `allMovements`
   - Implements MotionController interface
   - Provides clean API for pattern-based motion

8. **LineFollowController** ([LineFollowController.h](src/LineFollowController.h))
   - Wraps `LineFollower`
   - Implements MotionController interface
   - Provides clean API for line following

9. **MotionManager** ([MotionManager.h](src/MotionManager.h))
   - State machine for motion control
   - Ensures only one controller active at a time
   - Clean mode switching: `switchMode(MOTION_PATTERN)` or `switchMode(MOTION_LINE_FOLLOWING)`
   - Emergency stop support

10. **Example main.cpp** ([main_new.cpp](src/main_new.cpp))
    - Demonstrates new architecture
    - Shows how to use MotionManager
    - Examples of mode switching

---

## Architecture Comparison

### Before (Old Architecture)

```
main.cpp
  ├── allMovements (patterns)
  │     └── DriveUnit
  │           └── Wheel : L298 (inheritance)
  │
  └── LineFollower (GPS/IMU)
        └── DriveUnit
              └── Wheel : L298 (inheritance)

Problems:
❌ Wheel inherits from L298 (conceptually wrong)
❌ No coordination between allMovements and LineFollower
❌ Can't easily switch modes
❌ No clear "who's in control"
```

### After (New Architecture)

```
main.cpp
  └── MotionManager (state machine)
        ├── PatternController
        │     └── allMovements
        │           └── DriveUnit
        │                 └── Wheel (HAS-A Motor)
        │
        └── LineFollowController
              └── LineFollower
                    └── DriveUnit
                          └── Wheel (HAS-A Motor)

Benefits:
✅ Wheel uses composition (HAS-A motor)
✅ Clear coordination via MotionManager
✅ Easy mode switching
✅ Always know which controller is active
✅ Extensible for new controllers
```

---

## How to Migrate

### Option 1: Gradual Migration (Recommended)

Keep old code working while testing new architecture.

**Step 1**: Test new Wheel class
```cpp
// In a test sketch:
#include "Wheel_new.h"
#include "L298.h"

L298 motor(5, 8, 9);
Wheel wheel(&motor);

void loop() {
   wheel.setWheelSpeed(500, 10);
   wheel.EmitNewSpeed();
}
```

**Step 2**: Test DriveUnit_new
```cpp
#include "DriveUnit_new.h"

Scheduler TS;
DriveUnit drive(&TS, 64);

void setup() {
   drive.setTargetSpeed(Speed50, Speed50, 1000);
}
```

**Step 3**: Test MotionManager
```cpp
// Use main_new.cpp as reference
// Test pattern mode first
// Then test line following mode
// Then test mode switching
```

**Step 4**: Replace old files once confident
```
Rename:
  Wheel.h         → Wheel_old.h
  driveunit.h     → driveunit_old.h
  main.cpp        → main_old.cpp

  Wheel_new.h     → Wheel.h
  DriveUnit_new.h → DriveUnit.h
  main_new.cpp    → main.cpp
```

---

### Option 2: Clean Break

Replace everything at once (riskier, but cleaner).

**Step 1**: Backup current code
```bash
git commit -am "Backup before refactoring"
# Or copy entire directory
```

**Step 2**: Replace files
```
Delete: Wheel.h (old inheritance version)
Rename: Wheel_new.h → Wheel.h

Delete: driveunit.h, driveunit.cpp
Rename: DriveUnit_new.h → DriveUnit.h
Create: DriveUnit.cpp (empty, all inline)

Replace: main.cpp with main_new.cpp
```

**Step 3**: Update includes in other files
```cpp
// In LineFollower.cpp:
#include "driveunit.h"  // Should now work with new DriveUnit
```

**Step 4**: Compile and test

---

## Using the New Architecture

### Basic Pattern Motion

```cpp
#include "MotionManager.h"

// Setup
PatternController patternController(&moves);
LineFollowController lineFollowController(&lineFollower);
MotionManager motionManager(&patternController, &lineFollowController);

// Use pattern motion
patternController.setPattern(CIRCLE);
motionManager.switchMode(MOTION_PATTERN);
```

### Line Following

```cpp
// Configure line
lineFollowController.setLine(Point2D(0, 0), Point2D(10, 0));
lineFollowController.setBaseSpeed(Speed50);

// Start following
motionManager.switchMode(MOTION_LINE_FOLLOWING);

// Check status
if (lineFollowController.isComplete()) {
   Serial.println("Line complete!");
}
```

### Mode Switching

```cpp
// Switch from pattern to line following
motionManager.switchMode(MOTION_LINE_FOLLOWING);
// Old controller stops automatically
// New controller starts automatically

// Check current mode
if (motionManager.getCurrentState() == MOTION_PATTERN) {
   // In pattern mode
}

// Emergency stop
motionManager.emergencyStop();
```

### Obstacle Avoidance Integration

```cpp
void loop() {
   TS.execute();

   // Check sonar
   if (SonarData.pull(_distance)) {
      if (_distance < sSonar::MMtoMeasure(150)) {
         // Obstacle detected - stop immediately
         motionManager.emergencyStop();

         // Then switch to avoidance
         // (when you implement ObstacleAvoidController)
         // motionManager.switchMode(MOTION_OBSTACLE_AVOID);
      }
   }
}
```

---

## File Reference

### Core Files (Keep)
- `globals.hpp` - Now with debug macros
- `motor.hpp` - Motor interface (unchanged)
- `L298.h`, `L298.cpp` - Motor driver (unchanged)
- `movePatterns.h` - Pattern definitions (unchanged)
- `GPSInterface.h` - GPS stub (unchanged)
- `IMUInterface.h` - IMU interface (unchanged)

### Old Files (Can Remove After Migration)
- `Wheel.h` - Old inheritance version
- `driveunit.h`, `driveunit.cpp` - Old version
- `main.cpp` - Old version

### New Files (To Use)
- `Wheel_new.h` → rename to `Wheel.h`
- `DriveUnit_new.h` → rename to `DriveUnit.h`
- `MotionController.h` - Interface
- `PatternController.h` - Pattern wrapper
- `LineFollowController.h` - Line following wrapper
- `MotionManager.h` - State machine
- `main_new.cpp` → rename to `main.cpp`

### Unchanged Files
- `LineFollower.h`, `LineFollower.cpp` - Works with new architecture
- `allMoves.h` - Works with new architecture
- `SensorSonar.h` - Unchanged
- `Serial_mon.h` - Unchanged
- `queue.h` - Unchanged

---

## Benefits Summary

### Code Quality
✅ Fixed critical speed bug
✅ Proper composition over inheritance
✅ Clear separation of concerns
✅ Debug output can be disabled

### Maintainability
✅ Easy to understand "who's in control"
✅ Simple mode switching
✅ Extensible for new controllers
✅ Better error handling

### Features
✅ State machine for motion control
✅ Clean API for all controllers
✅ Emergency stop support
✅ Mode transition safety

---

## Testing Checklist

### Phase 1 Tests
- [ ] Speed increment bug fixed (acceleration goes correct direction)
- [ ] Debug output works
- [ ] Debug can be disabled (`DEBUG_ENABLED 0`)

### Phase 2 Tests
- [ ] New Wheel class works with L298
- [ ] New DriveUnit creates wheels correctly
- [ ] Pattern mode works via MotionManager
- [ ] Line following works via MotionManager
- [ ] Mode switching works (pattern → line)
- [ ] Emergency stop works
- [ ] Only one controller active at a time

---

## Next Steps

### Immediate
1. Test new architecture with `main_new.cpp`
2. Verify all functionality still works
3. Migrate when confident

### Future Enhancements
1. **ObstacleAvoidController** - Implement MOTION_OBSTACLE_AVOID
2. **ReturnToBaseController** - Return to charging station
3. **CoverageController** - Systematic lawn coverage
4. **Telemetry** - Add status reporting
5. **Configuration** - Save/load controller parameters

---

## Rollback Plan

If something goes wrong:

### If Using Gradual Migration
```bash
# Just delete new files
rm Wheel_new.h DriveUnit_new.h MotionController.h
rm PatternController.h LineFollowController.h MotionManager.h main_new.cpp

# Old files still work
```

### If Using Clean Break
```bash
# Restore from backup
git reset --hard HEAD^
# Or restore from directory backup
```

---

## Questions?

**Q: Do I have to use MotionManager?**
A: No, you can still use PatternController and LineFollowController directly. MotionManager just makes mode switching cleaner.

**Q: Can I mix old and new code?**
A: Mostly yes. New controllers work with old DriveUnit. But for full benefits, use new Wheel and DriveUnit.

**Q: What if I only want the bug fix?**
A: Just apply Phase 1 changes (speed bug fix and debug macros). Skip Phase 2.

**Q: How do I add a new controller?**
A: Create a class that implements MotionController interface, wrap your logic, add to MotionManager.

**Q: Can I still use obstacle avoidance?**
A: Yes! MotionManager can stop current controller and switch to avoidance mode.

---

**Status**: Phase 1 and Phase 2 complete and ready to test!
