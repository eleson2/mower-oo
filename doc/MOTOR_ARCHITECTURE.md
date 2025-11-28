# Motor Architecture

## Overview
The motor control system uses a clean abstraction layer that separates hardware drivers from motion control logic. This design allows easy swapping of motor drivers without modifying higher-level code.

## Class Hierarchy

```
┌─────────────────────────────────────────────────────────────┐
│                        Motor (interface)                     │
│  - Pure virtual base class                                   │
│  - Standardized speed range: -1023 to +1023                  │
│  - Methods: move(), stop(), reset(), getSpeed()              │
└─────────────────────────────────────────────────────────────┘
                            ▲
                            │ implements
            ┌───────────────┼───────────────┬─────────────────┐
            │               │               │                 │
    ┌───────┴──────┐  ┌────┴─────┐  ┌──────┴────────┐  ┌─────┴──────┐
    │    L298      │  │HighPower │  │ VirtualMotor  │  │  Future    │
    │              │  │ HBridge  │  │  (Testing)    │  │  Drivers   │
    │ - PWM pins   │  │          │  │               │  │            │
    │ - Direction  │  │ - 60A    │  │ - Debug logs  │  │            │
    │   control    │  │ - 31kHz  │  │ - No hardware │  │            │
    └──────────────┘  └──────────┘  └───────────────┘  └────────────┘
```

## Composition Structure

```
┌──────────────────────────────────────────────────────────────────┐
│                         DriveUnit (Task)                         │
│  - Coordinates left and right wheels                             │
│  - Smooth speed transitions over time                            │
│  - TaskScheduler integration                                     │
│                                                                  │
│  ┌────────────────────┐              ┌────────────────────┐     │
│  │   Wheel (left)     │              │  Wheel (right)     │     │
│  │                    │              │                    │     │
│  │ - Speed interp.    │              │ - Speed interp.    │     │
│  │ - Smooth accel.    │              │ - Smooth accel.    │     │
│  │                    │              │                    │     │
│  │  ┌──────────────┐  │              │  ┌──────────────┐  │     │
│  │  │   Motor*     │  │              │  │   Motor*     │  │     │
│  │  │  (interface) │  │              │  │  (interface) │  │     │
│  │  │              │  │              │  │              │  │     │
│  │  │  points to:  │  │              │  │  points to:  │  │     │
│  │  │  L298 or     │  │              │  │  L298 or     │  │     │
│  │  │  HighPower   │  │              │  │  HighPower   │  │     │
│  │  │  or Virtual  │  │              │  │  or Virtual  │  │     │
│  │  └──────────────┘  │              │  └──────────────┘  │     │
│  └────────────────────┘              └────────────────────┘     │
└──────────────────────────────────────────────────────────────────┘
```

## Object Lifecycle

### Production Configuration (Hardware)
```
main.cpp:
  ┌─> L298 leftMotor(LEFTENABLE, LEFTIN1, LEFTIN2)
  │
  ├─> L298 rightMotor(RIGHTENABLE, RIGHTIN1, RIGHTIN2)
  │
  └─> DriveUnit driveUnit(&scheduler, WheelUpdateRate)
        │
        └─> Internally creates:
              - Wheel(leftMotor)
              - Wheel(rightMotor)
```

### Testing Configuration (No Hardware)
```
main.cpp:
  ┌─> VirtualMotor leftMotor("Left")
  │
  ├─> VirtualMotor rightMotor("Right")
  │
  └─> DriveUnit driveUnit(&leftMotor, &rightMotor, &scheduler, WheelUpdateRate)
        │
        └─> Internally creates:
              - Wheel(leftMotor)
              - Wheel(rightMotor)
```

### High-Power Configuration (60A Driver)
```
main.cpp:
  ┌─> HighPowerHBridgeMotor leftMotor(PWM_PIN_L, A1_PIN_L, A2_PIN_L)
  │
  ├─> HighPowerHBridgeMotor rightMotor(PWM_PIN_R, A1_PIN_R, A2_PIN_R)
  │
  └─> DriveUnit driveUnit(&leftMotor, &rightMotor, &scheduler, WheelUpdateRate)
        │
        └─> Internally creates:
              - Wheel(leftMotor)
              - Wheel(rightMotor)
```

## Layer Responsibilities

### Layer 1: Motor Interface (motor.hpp)
**Responsibility:** Define standard API for all motor drivers
- **Speed range:** -1023 to +1023 (hardware-independent)
- **Pure virtual:** No implementation, only interface
- **No dependencies:** Arduino.h only

**Files:**
- `src/motor.hpp`

### Layer 2: Hardware Drivers (Thin Shims)
**Responsibility:** Map Motor interface to specific hardware
- **Hardware-specific:** Pin control, PWM setup
- **Minimal logic:** Only what's needed for the hardware
- **Easy to replace:** Implement Motor interface, done

**Files:**
- `src/L298.h` / `src/L298.cpp` (L298N dual H-bridge)
- `src/HighPowerHBridgeMotor.hpp` / `src/HighPowerHBridgeMotor.cpp` (60A H-bridge)
- `src/VirtualMotor.h` (Testing/debugging)

### Layer 3: Motion Control
**Responsibility:** Speed interpolation and coordination
- **Hardware-agnostic:** Uses Motor* interface
- **Smooth motion:** Gradual acceleration/deceleration
- **Differential drive:** Coordinates left/right wheels

**Files:**
- `src/Wheel.h` (Single wheel with smooth speed transitions)
- `src/DriveUnit.h` (Coordinates both wheels, TaskScheduler integration)

## Speed Flow

```
User Code
   │ setTargetSpeed(500, 500, 2000ms)
   ▼
DriveUnit
   │ Calculates iterations
   │ Enables Task
   ▼
Wheel (left)          Wheel (right)
   │ setWheelSpeed()     │ setWheelSpeed()
   │ Fixed-point math    │ Fixed-point math
   │                     │
   │ Every WheelUpdateRate (64ms):
   │ EmitNewSpeed()      │ EmitNewSpeed()
   │   cur += step       │   cur += step
   ▼                     ▼
Motor->move(speed)    Motor->move(speed)
   │                     │
   ▼                     ▼
L298 / HighPower / Virtual
   │ Maps -1023..1023 to PWM 0..255
   │ Sets direction pins
   ▼
Hardware / Debug Output
```

## Key Design Principles

### 1. Dependency Injection
DriveUnit accepts `Motor*` pointers, allowing runtime selection of motor implementation:
```cpp
// Option A: DriveUnit creates its own L298 motors
DriveUnit drive(&scheduler, 64);

// Option B: Inject custom motor implementations
DriveUnit drive(&customLeft, &customRight, &scheduler, 64);
```

### 2. Interface Segregation
Each layer has a single, focused responsibility:
- **Motor:** Hardware abstraction only
- **Wheel:** Speed interpolation only
- **DriveUnit:** Wheel coordination only

### 3. Open/Closed Principle
**Adding a new motor driver requires:**
1. Create new class implementing `Motor` interface
2. Implement 4 methods: `move()`, `stop()`, `reset()`, `getSpeed()`
3. Map -1023..1023 to hardware-specific values
4. **Zero changes** to Wheel, DriveUnit, or other code

### 4. Hardware Independence
All motion control code (Wheel, DriveUnit) works with the standardized -1023..1023 range. The hardware driver handles the specific PWM mapping.

## Standardized Speed Range

**Why -1023 to +1023?**
- **Efficient mapping:** 1023 >> 2 = 255 (8-bit PWM)
- **Bit-shift operation:** Much faster than division on 8-bit MCUs
- **Symmetric:** Same range for forward and reverse
- **Hardware-independent:** All drivers use this internally

**Conversion example:**
```cpp
// In L298.cpp or HighPowerHBridgeMotor.cpp:
void move(int speed) {
    speed = constrain(speed, -1023, 1023);
    uint8_t pwm = abs(speed) >> 2;  // Fast: divide by 4
    // Set direction and PWM
}
```

## Adding a New Motor Driver

**Step-by-step:**

1. **Create header file:** `src/MyMotorDriver.hpp`
```cpp
#include "motor.hpp"

class MyMotorDriver : public Motor {
public:
    MyMotorDriver(byte pin1, byte pin2);
    void move(int speed) override;
    void stop() override;
    void reset() override;
    int getSpeed() override;
private:
    // Hardware-specific members
};
```

2. **Implement methods:** `src/MyMotorDriver.cpp`
```cpp
void MyMotorDriver::move(int speed) {
    speed = constrain(speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
    // Map to your hardware's requirements
    // Set direction and PWM
}
```

3. **Use in main.cpp:**
```cpp
MyMotorDriver leftMotor(pin1, pin2);
MyMotorDriver rightMotor(pin3, pin4);
DriveUnit drive(&leftMotor, &rightMotor, &scheduler, 64);
```

**That's it!** No changes to Wheel, DriveUnit, or any other code.

## Files Reference

| File | Layer | Purpose |
|------|-------|---------|
| `motor.hpp` | Interface | Pure virtual Motor base class |
| `L298.h/cpp` | Driver | L298N dual H-bridge implementation |
| `HighPowerHBridgeMotor.hpp/cpp` | Driver | 60A high-power H-bridge |
| `VirtualMotor.h` | Driver | Testing/debugging implementation |
| `Wheel.h` | Control | Single wheel speed interpolation |
| `DriveUnit.h` | Control | Differential drive coordination |
| `globals.hpp` | Config | Pin assignments, speed constants |

## Related Documentation
- `src/motor.hpp` - Motor interface specification with detailed comments
- `src/Wheel.h` - Wheel speed interpolation algorithm
- `src/DriveUnit.h` - Differential drive coordination
