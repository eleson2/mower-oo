#ifndef _MOTION_MANAGER_H
#define _MOTION_MANAGER_H

#include "MotionController.h"
#include "PatternController.h"
#include "LineFollowController.h"

// Motion Manager - coordinates different motion controllers
// Ensures only one controller is active at a time
// Provides clean state transitions
class MotionManager {
private:
   MotionController* _activeController;
   PatternController* _patternController;
   LineFollowController* _lineFollowController;
   MotionState _currentState;

public:
   MotionManager(PatternController* patternCtrl, LineFollowController* lineFollowCtrl)
      : _activeController(nullptr),
        _patternController(patternCtrl),
        _lineFollowController(lineFollowCtrl),
        _currentState(MOTION_IDLE)
   {
   }

   // Switch to a new motion mode
   void switchMode(MotionState newState) {
      // Stop current controller
      if (_activeController) {
         DEBUG_PRINT("Stopping ");
         DEBUG_PRINTLN(_activeController->getName());
         _activeController->stop();
         _activeController = nullptr;
      }

      // Switch to new controller
      switch (newState) {
         case MOTION_PATTERN:
            _activeController = _patternController;
            DEBUG_PRINTLN("Switching to Pattern mode");
            break;

         case MOTION_LINE_FOLLOWING:
            _activeController = _lineFollowController;
            DEBUG_PRINTLN("Switching to Line Following mode");
            break;

         case MOTION_OBSTACLE_AVOID:
            // Could add ObstacleAvoidController here
            DEBUG_PRINTLN("Obstacle avoidance not yet implemented");
            _activeController = nullptr;
            break;

         case MOTION_EMERGENCY_STOP:
         case MOTION_IDLE:
         default:
            DEBUG_PRINTLN("Switching to Idle mode");
            _activeController = nullptr;
            break;
      }

      _currentState = newState;

      // Start new controller
      if (_activeController) {
         DEBUG_PRINT("Starting ");
         DEBUG_PRINTLN(_activeController->getName());
         _activeController->start();
      }
   }

   // Emergency stop - immediately halt all motion
   void emergencyStop() {
      DEBUG_PRINTLN("EMERGENCY STOP!");
      if (_activeController) {
         _activeController->stop();
      }
      _activeController = nullptr;
      _currentState = MOTION_EMERGENCY_STOP;
   }

   // Get current state
   MotionState getCurrentState() const {
      return _currentState;
   }

   // Get active controller
   MotionController* getActiveController() {
      return _activeController;
   }

   // Check if any controller is active
   bool isActive() const {
      return _activeController != nullptr && _activeController->isActive();
   }

   // Get pattern controller (for direct access if needed)
   PatternController* getPatternController() { return _patternController; }

   // Get line follow controller (for direct access if needed)
   LineFollowController* getLineFollowController() { return _lineFollowController; }

   // Update active controller
   void update() {
      if (_activeController) {
         _activeController->update();
      }
   }
};

#endif
