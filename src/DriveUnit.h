#ifndef DRIVEUNIT_H
#define DRIVEUNIT_H

#include "globals.hpp"
#include "Wheel.h"
#include "L298.h"
#include "VirtualMotor.h"
#include <TaskSchedulerDeclarations.h>

// DriveUnit - coordinates left and right wheel motors with smooth speed transitions
class DriveUnit : public Task {
private:
   Motor* _leftMotor;     // Left motor driver (can be L298 or virtual)
   Motor* _rightMotor;    // Right motor driver (can be L298 or virtual)
   Wheel* _leftWheel;     // Left wheel (uses composition)
   Wheel* _rightWheel;    // Right wheel (uses composition)
   bool _ownsMotors;

public:
   // Default constructor: create hardware L298 drivers and own them
   DriveUnit(Scheduler* aS, unsigned int mSec)
      : Task(mSec, TASK_FOREVER, aS, false),
        _leftMotor(nullptr), _rightMotor(nullptr), _leftWheel(nullptr), _rightWheel(nullptr), _ownsMotors(true)
   {
      // allocate hardware drivers
      _leftMotor = new L298(LEFTENABLE, LEFTIN1, LEFTIN2);
      _rightMotor = new L298(RIGHTENABLE, RIGHTIN1, RIGHTIN2);
      _leftWheel = new Wheel(_leftMotor);
      _rightWheel = new Wheel(_rightMotor);
   };

   // Injection constructor: supply Motor implementations (virtual or hardware)
   DriveUnit(Motor* leftMotor, Motor* rightMotor, Scheduler* aS, unsigned int mSec)
      : Task(mSec, TASK_FOREVER, aS, false),
        _leftMotor(leftMotor), _rightMotor(rightMotor), _leftWheel(nullptr), _rightWheel(nullptr), _ownsMotors(false)
   {
      _leftWheel = new Wheel(_leftMotor);
      _rightWheel = new Wheel(_rightMotor);
   };

   ~DriveUnit() { 
      // cleanup wheels
      if (_leftWheel) { delete _leftWheel; _leftWheel = nullptr; }
      if (_rightWheel) { delete _rightWheel; _rightWheel = nullptr; }
      // cleanup motors if we own them
      if (_ownsMotors) {
         if (_leftMotor) { delete _leftMotor; _leftMotor = nullptr; }
         if (_rightMotor) { delete _rightMotor; _rightMotor = nullptr; }
      }
   };

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

      _leftWheel->setWheelSpeed(leftSpeed, iterations);
      _rightWheel->setWheelSpeed(rightSpeed, iterations);
      setIterations(iterations);
      enable();
   };

   // Task callbacks
   bool Callback() override {
      _leftWheel->EmitNewSpeed();
      _rightWheel->EmitNewSpeed();
      return true;
   };

   bool OnEnable() override {
      DEBUG_PRINTLN("DriveUnit OnEnable:");
      _leftWheel->EmitNewSpeed();
      _rightWheel->EmitNewSpeed();
      return true;
   };

   void OnDisable() override {
      DEBUG_PRINTLN("EmitTargetSpeed:");
      _leftWheel->EmitTargetSpeed();
      _rightWheel->EmitTargetSpeed();
   };

   // Get current speeds
   int getLeftSpeed() const { return _leftWheel ? _leftWheel->getCurrentSpeed() : 0; }
   int getRightSpeed() const { return _rightWheel ? _rightWheel->getCurrentSpeed() : 0; }

   // Stop both wheels
   void stopWheels() {
      if (_leftWheel) _leftWheel->stop();
      if (_rightWheel) _rightWheel->stop();
      disable();
   };
};

#endif
