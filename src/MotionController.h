#ifndef _MOTION_CONTROLLER_H
#define _MOTION_CONTROLLER_H

#include "Arduino.h"

// Motion state for state machine
enum MotionState {
   MOTION_IDLE,              // Not moving
   MOTION_PATTERN,           // Following predefined pattern
   MOTION_LINE_FOLLOWING,    // GPS/IMU line following
   MOTION_OBSTACLE_AVOID,    // Avoiding obstacle
   MOTION_EMERGENCY_STOP     // Emergency stop
};

// Abstract base class for all motion controllers
// Provides common interface for different motion control strategies
class MotionController {
public:
   virtual ~MotionController() {}

   // Start this controller
   virtual void start() = 0;

   // Stop this controller
   virtual void stop() = 0;

   // Check if controller is currently active
   virtual bool isActive() const = 0;

   // Update controller (called periodically by scheduler)
   virtual void update() = 0;

   // Get controller name (for debugging)
   virtual const char* getName() const = 0;

   // Get current motion state
   virtual MotionState getState() const = 0;
};

#endif
