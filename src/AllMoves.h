#ifndef ALLMOVES_H
#define ALLMOVES_H

#include "globals.hpp"
#include <TaskSchedulerDeclarations.h>

class AllMovements : public Task {
private:
#include <movePatterns.h>

    /* data */
   inline static movement *currMove = nullptr;
   inline static motorSpeedCallback AdjustSpeedCallback = nullptr;
   CurrentMotion CurrMotion = CONTINUOUS;

public:
   AllMovements(Scheduler* aS, motorSpeedCallback f);
   ~AllMovements();

   bool Callback() override;
   void setCurrentPattern(CurrentMotion _CM);
   void setCallback(motorSpeedCallback f);
   inline CurrentMotion CurrentPattern() { return CurrMotion; };
};

inline AllMovements::AllMovements(Scheduler* aS, motorSpeedCallback f)
          : Task(1, TASK_FOREVER, aS, false) {
   setIterations(1);
   AdjustSpeedCallback = f;
}

inline AllMovements::~AllMovements() {
}

inline bool AllMovements::Callback() {
   DEBUG_PRINT("Moves task CB  ");
   DEBUG_PRINTLN(currMove->mSec);
   if (currMove->mSec == 0) {
       currMove = Continous; //When at last action, run straight.
       DEBUG_PRINTLN("EOF current move, going continous");
   }
   AdjustSpeedCallback(*currMove);
   setInterval(currMove->mSec);
   DEBUG_PRINT("currMove->mSec  :");
   DEBUG_PRINTLN(currMove->mSec);
   currMove++;
   DEBUG_PRINT("Next move ->mSec  :");
   DEBUG_PRINTLN(currMove->mSec);
   restartDelayed();
   return true;
};

inline void AllMovements::setCurrentPattern(CurrentMotion _CM) {
   AllMovements::CurrMotion = _CM;
   DEBUG_PRINT("SetCurMotion:  ");
   DEBUG_PRINTLN(_CM);

   switch (_CM) {
   case CONTINUOUS:
      currMove = Continous;
      break;
   case CHARGER_BACKOUT:
      currMove = ChargerBackout;
      break;
   case BWF_LEFT:
      currMove = BWFLeft;
      break;
   case BWF_RIGHT:
      currMove = BWFRight;
      break;
   case CIRCLE:
      currMove = Circle;
      break;
   case TURN_LEFT:
      currMove = TurnLeft;
      break;
   case SLOW_DOWN:
      currMove = SlowDown;
      break;
   case AVOID_OBSTACLE:
      currMove = AvoidObstacle;
      break;
   default:
      DEBUG_PRINTLN("Default move!");
      currMove = Continous;
   };
   restart();
};

#endif