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
   int  TargetSpeed;     // integer speed (-1023..+1023)
   int  CurSpeed;        // integer speed currently applied
   // Fixed-point accumulator for smooth interpolation
   // SCALE = 1024 chosen to be cheap on small MCUs (power of two)
   static const int SCALE = 1024;
   int32_t cur_acc;      // CurSpeed * SCALE
   int32_t target_acc;   // TargetSpeed * SCALE
   int32_t step_acc;     // per-iteration increment in accumulator units

public:
   // Constructor - takes a motor driver as parameter (dependency injection)
   Wheel(Motor* motor) : _motor(motor) {
      CurSpeed = 0;
      TargetSpeed = 0;
      cur_acc = 0;
      target_acc = 0;
      step_acc = 0;
      if (_motor) {
         _motor->reset();
      }
   };

   // Emit interpolated speed
   void EmitNewSpeed() {
      // advance accumulator and compute integer speed
      cur_acc += step_acc;
      int newSpeed = (int)((cur_acc + (SCALE/2)) / SCALE); // rounding
      // Only update motor if changed
      if (newSpeed != CurSpeed) {
         CurSpeed = newSpeed;
         if (_motor) {
            _motor->move(CurSpeed);
         }
      }
   };

   // Emit target speed (skip interpolation)
   void EmitTargetSpeed() {
      // force exact target
      CurSpeed = TargetSpeed;
      cur_acc = (int32_t)CurSpeed * SCALE;
      target_acc = (int32_t)TargetSpeed * SCALE;
      step_acc = 0;
      if (_motor) {
         _motor->move(TargetSpeed);
      }
   };

   // Set target speed with number of interpolation steps
   void setWheelSpeed(int Speed, int iterations) {
      TargetSpeed = Speed;
      // Prepare fixed-point accumulator values
      target_acc = (int32_t)TargetSpeed * SCALE;
      cur_acc = (int32_t)CurSpeed * SCALE;

      if (iterations <= 1) iterations = 2;
      step_acc = (target_acc - cur_acc) / iterations;  // scaled increment per tick

      // If step_acc is zero due to small delta/large SCALE, fall back to at least move by 1 LSB
      if (step_acc == 0 && target_acc != cur_acc) {
         step_acc = (target_acc > cur_acc) ? 1 : -1;
      }

      DEBUG_PRINT("StepAcc: ");
      DEBUG_PRINT(step_acc);
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
      cur_acc = 0;
      target_acc = 0;
      step_acc = 0;
      if (_motor) {
         _motor->reset();
      }
   };

   // Stop wheel
   void stop() {
      CurSpeed = 0;
      TargetSpeed = 0;
      cur_acc = 0;
      target_acc = 0;
      step_acc = 0;
      if (_motor) {
         _motor->stop();
      }
   };
};

#endif
