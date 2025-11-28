#ifndef _PATTERN_CONTROLLER_H
#define _PATTERN_CONTROLLER_H

#include "MotionController.h"
#include "allMoves.h"

// Pattern-based motion controller
// Wraps allMovements to provide MotionController interface
class PatternController : public MotionController {
private:
   allMovements* _movements;
   bool _isActive;
   MotionState _state;

public:
   PatternController(allMovements* movements)
      : _movements(movements), _isActive(false), _state(MOTION_IDLE)
   {
   }

   void start() override {
      if (_movements) {
         _movements->enable();
         _isActive = true;
         _state = MOTION_PATTERN;
         DEBUG_PRINTLN("PatternController started");
      }
   }

   void stop() override {
      if (_movements) {
         _movements->disable();
         _isActive = false;
         _state = MOTION_IDLE;
         DEBUG_PRINTLN("PatternController stopped");
      }
   }

   bool isActive() const override {
      return _isActive && _movements && _movements->isEnabled();
   }

   void update() override {
      // allMovements handles its own updates via TaskScheduler
      // Nothing needed here
   }

   const char* getName() const override {
      return "PatternController";
   }

   MotionState getState() const override {
      return _state;
   }

   // Pattern-specific methods
   void setPattern(CurrentMotion pattern) {
      if (_movements) {
         _movements->setCurrentPattern(pattern);
      }
   }

   CurrentMotion getCurrentPattern() const {
      return _movements ? _movements->CurrentPattern() : CONTINOUS;
   }
};

#endif
