#ifndef _LINE_FOLLOW_CONTROLLER_H
#define _LINE_FOLLOW_CONTROLLER_H

#include "MotionController.h"
#include "LineFollower.h"

// GPS/IMU-based line following controller
// Wraps LineFollower to provide MotionController interface
class LineFollowController : public MotionController {
private:
   LineFollower* _lineFollower;
   bool _isActive;
   MotionState _state;

public:
   LineFollowController(LineFollower* lineFollower)
      : _lineFollower(lineFollower), _isActive(false), _state(MOTION_IDLE)
   {
   }

   void start() override {
      if (_lineFollower) {
         _lineFollower->enable();
         _isActive = true;
         _state = MOTION_LINE_FOLLOWING;
         DEBUG_PRINTLN("LineFollowController started");
      }
   }

   void stop() override {
      if (_lineFollower) {
         _lineFollower->disable();
         _isActive = false;
         _state = MOTION_IDLE;
         DEBUG_PRINTLN("LineFollowController stopped");
      }
   }

   bool isActive() const override {
      return _isActive && _lineFollower && _lineFollower->isEnabled();
   }

   void update() override {
      // LineFollower handles its own updates via TaskScheduler
      // Nothing needed here
   }

   const char* getName() const override {
      return "LineFollowController";
   }

   MotionState getState() const override {
      return _state;
   }

   // Line following-specific methods
   void setLine(Point2D start, Point2D end) {
      if (_lineFollower) {
         _lineFollower->setLine(start, end);
      }
   }

   bool isComplete() const {
      return _lineFollower ? _lineFollower->isComplete() : true;
   }

   float getCrossTrackError() {
      return _lineFollower ? _lineFollower->getCrossTrackError() : 0.0f;
   }

   void setCrossTrackGain(float gain) {
      if (_lineFollower) _lineFollower->setCrossTrackGain(gain);
   }

   void setHeadingGain(float gain) {
      if (_lineFollower) _lineFollower->setHeadingGain(gain);
   }

   void setLookaheadDistance(float distance) {
      if (_lineFollower) _lineFollower->setLookaheadDistance(distance);
   }

   void setBaseSpeed(wheelSpeed speed) {
      if (_lineFollower) _lineFollower->setBaseSpeed(speed);
   }

   void reset() {
      if (_lineFollower) _lineFollower->reset();
   }
};

#endif
