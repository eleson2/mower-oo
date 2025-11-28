#ifndef DRIVEUNIT_H
#define DRIVEUNIT_H

#include "globals.hpp"
#include "Wheel.h"
#include "L298.h"
#include <TaskSchedulerDeclarations.h>

// DriveUnit - coordinates left and right wheel motors with smooth speed transitions
class DriveUnit : public Task {
private:
   L298 _leftMotor;     // Left motor driver
   L298 _rightMotor;    // Right motor driver
   Wheel _leftWheel;    // Left wheel (uses composition)
   Wheel _rightWheel;   // Right wheel (uses composition)

public:
   DriveUnit(Scheduler* aS, unsigned int mSec)
      : Task(mSec, TASK_FOREVER, aS, false),
        _leftMotor(LEFTENABLE, LEFTIN1, LEFTIN2),
        _rightMotor(RIGHTENABLE, RIGHTIN1, RIGHTIN2),
        _leftWheel(&_leftMotor),
        _rightWheel(&_rightMotor)
   {
   };

   ~DriveUnit() {};

   // Set target speeds with smooth transition over time
   void setTargetSpeed(int leftSpeed, int rightSpeed, int mSecToReachSpeed) {
      int iterations = mSecToReachSpeed / WheelUpdateRate;
      if (iterations <= 1) iterations = 2;

      DEBUG_PRINT("Set_T_S: ");
      DEBUG_PRINT(leftSpeed);
      DEBUG_PRINT(" ");
      DEBUG_PRINT(rightSpeed);
      DEBUG_PRINT(" ");
      DEBUG_PRINT(mSecToReachSpeed);
      DEBUG_PRINT(" ");
      DEBUG_PRINTLN(iterations);

      _leftWheel.setWheelSpeed(leftSpeed, iterations);
      _rightWheel.setWheelSpeed(rightSpeed, iterations);
      setIterations(iterations);
      enable();
   };

   // Task callbacks
   bool Callback() override {
      _leftWheel.EmitNewSpeed();
      _rightWheel.EmitNewSpeed();
      return true;
   };

   bool OnEnable() override {
      DEBUG_PRINTLN("DriveUnit OnEnable:");
      _leftWheel.EmitNewSpeed();
      _rightWheel.EmitNewSpeed();
      return true;
   };

   void OnDisable() override {
      DEBUG_PRINTLN("EmitTargetSpeed:");
      _leftWheel.EmitTargetSpeed();
      _rightWheel.EmitTargetSpeed();
   };

   // Get current speeds
   int getLeftSpeed() const { return _leftWheel.getCurrentSpeed(); }
   int getRightSpeed() const { return _rightWheel.getCurrentSpeed(); }

   // Stop both wheels
   void stopWheels() {
      _leftWheel.stop();
      _rightWheel.stop();
      disable();
   };
};

#endif
