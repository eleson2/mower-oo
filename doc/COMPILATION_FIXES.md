# Compilation Fixes Applied

## Issues Fixed

### Issue 1: Multiple Definition Errors

**Problem**: `allMoves.h` had class implementations in the header file without `inline` keyword, causing multiple definition errors when included in multiple translation units.

**Root Cause**:
- Missing include guards
- Function implementations in header without `inline`
- Every .cpp file that included `allMoves.h` got a copy of the implementations
- Linker found multiple definitions and failed

**Errors**:
```
src/allMoves.h:4:7: error: redefinition of 'class allMovements'
src/allMoves.h:23:1: error: redefinition of 'allMovements::allMovements(...)'
src/allMoves.h:29:1: error: redefinition of 'allMovements::~allMovements()'
src/allMoves.h:32:6: error: redefinition of 'bool allMovements::Callback()'
src/allMoves.h:50:6: error: redefinition of 'void allMovements::setCurrentPattern(...)'
```

**Solution Applied**:
1. Added include guards (`#ifndef _ALL_MOVES_H`)
2. Marked all implementations as `inline`

**Files Modified**:
- [src/allMoves.h](src/allMoves.h)

**Changes**:
```cpp
// BEFORE (caused errors):
class allMovements : public Task { ... };

allMovements::allMovements(...) { ... }
bool allMovements::Callback() { ... }
void allMovements::setCurrentPattern(...) { ... }

// AFTER (fixed):
#ifndef _ALL_MOVES_H
#define _ALL_MOVES_H

class allMovements : public Task { ... };

inline allMovements::allMovements(...) { ... }
inline bool allMovements::Callback() { ... }
inline void allMovements::setCurrentPattern(...) { ... }

#endif
```

---

### Issue 2: Duplicate TaskScheduler Include Definitions

**Problem**: `driveunit.h` had its own `#define _TASK_OO_CALLBACKS` which conflicted with the one in `globals.hpp`.

**Root Cause**:
- Multiple places defining `_TASK_OO_CALLBACKS`
- Each triggers TaskScheduler to include implementation code
- Linker finds duplicate symbols

**Errors**:
```
src/driveunit.h:8:7: error: redefinition of 'class DriveUnit'
```

**Solution Applied**:
- Removed duplicate `#define _TASK_OO_CALLBACKS`
- Added `#include "globals.hpp"` which already has the define

**Files Modified**:
- [src/driveunit.h](src/driveunit.h)

**Changes**:
```cpp
// BEFORE:
#ifndef _DRIVEUNIT_
#define _DRIVEUNIT_

#define _TASK_OO_CALLBACKS  // ← Duplicate!

#include <Wheel.h>

// AFTER:
#ifndef _DRIVEUNIT_
#define _DRIVEUNIT_

#include "globals.hpp"  // ← Already has _TASK_OO_CALLBACKS
#include <Wheel.h>
```

---

## Pattern Used: Inline Functions in Headers

When you have function implementations in header files (common in template-heavy or small class libraries), you have two options:

### Option 1: Move to .cpp file (Preferred for large functions)
```cpp
// MyClass.h
class MyClass {
   void doSomething();  // Declaration only
};

// MyClass.cpp
void MyClass::doSomething() {  // Implementation
   // code here
}
```

### Option 2: Mark as inline (Used here for small functions)
```cpp
// MyClass.h
class MyClass {
   void doSomething();
};

inline void MyClass::doSomething() {  // inline = OK in header
   // code here
}
```

**Why `inline` works**:
- The `inline` keyword tells the linker "it's OK if you see this definition multiple times"
- Linker will pick one and discard the rest (One Definition Rule exemption)
- Common for small functions that benefit from inlining

---

## TaskScheduler Include Pattern

The correct pattern for TaskScheduler in this codebase:

### In globals.hpp (ONCE)
```cpp
#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>
```

### In header files (Task-based classes)
```cpp
#include "globals.hpp"  // Gets _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>  // Declarations only
```

### In main.cpp (Implementation)
```cpp
#include "globals.hpp"  // Gets _TASK_OO_CALLBACKS
#include <TaskScheduler.h>  // Full implementation
```

**Key Rule**: Only ONE file should include `<TaskScheduler.h>` - that's `main.cpp`. All other files use `<TaskSchedulerDeclarations.h>`.

---

## Verification

After these fixes, the following should compile cleanly:
- `src/allMoves.h` - No multiple definition errors
- `src/driveunit.h` - No duplicate Task symbols
- `src/LineFollower.h` - Uses TaskSchedulerDeclarations correctly
- `src/main.cpp` - Links everything together

---

## Summary

✅ **Fixed**: allMoves.h multiple definition errors (added inline)
✅ **Fixed**: driveunit.h duplicate _TASK_OO_CALLBACKS (removed duplicate)
✅ **Fixed**: TaskScheduler include pattern (consistent usage)

The codebase should now compile without linker errors!
