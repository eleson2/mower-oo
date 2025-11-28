#ifndef WHEEL_H
#define WHEEL_H

#include "globals.hpp"
#include "Arduino.h"
#include "motor.hpp"

// Wheel class - manages speed interpolation for a single wheel
// Uses composition (HAS-A motor) instead of inheritance (IS-A motor)
class Wheel {
private:
   Motor* _motor;         // Motor driver (L298, etc.) - composition!
   int  TargetSpeed;
   int  CurSpeed;
   int  SpeedIncrement;

public:
   // Constructor - takes a motor driver as parameter (dependency injection)
   Wheel(Motor* motor) : _motor(motor) {
      CurSpeed = 0;
      TargetSpeed = 0;
      SpeedIncrement = 0;
      if (_motor) {
         _motor->reset();
      }
   };

   // Emit interpolated speed
   void EmitNewSpeed() {
      CurSpeed += SpeedIncrement;
      if (_motor) {
         _motor->move(CurSpeed);
      }
   };

   // Emit target speed (skip interpolation)
   void EmitTargetSpeed() {
      if (_motor) {
         _motor->move(TargetSpeed);
      }
   };

   // Set target speed with number of interpolation steps
   void setWheelSpeed(int Speed, int iterations) {
      TargetSpeed = Speed;
      SpeedIncrement = (TargetSpeed - CurSpeed) / iterations;  // Fixed: was backwards!

      DEBUG_PRINT("SpeedIncr: ");
      DEBUG_PRINT(SpeedIncrement);
      DEBUG_PRINT("   Iter: ");
      DEBUG_PRINTLN(iterations);
   };

   // Get current speed
   int getCurrentSpeed() const { return CurSpeed; }

   // Get target speed
   int getTargetSpeed() const { return TargetSpeed; }

   // Reset wheel
   void reset() {
      CurSpeed = 0;
      TargetSpeed = 0;
      SpeedIncrement = 0;
      if (_motor) {
         _motor->reset();
      }
   };

   // Stop wheel
   void stop() {
      CurSpeed = 0;
      TargetSpeed = 0;
      SpeedIncrement = 0;
      if (_motor) {
         _motor->stop();
      }
   };
};

#endif
