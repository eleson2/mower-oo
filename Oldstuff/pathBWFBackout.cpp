#include <Arduino.h>
#include <./patterns/pathBWFBackout.h>


mBWFBackout::mBWFBackout(Scheduler* aS, unsigned int mSec) 
          : DrivePattern(mSec,TASK_FOREVER,aS,false) {
};

mBWFBackout::~mBWFBackout(){}; 

bool mBWFBackout::Callback() { 

   switch(seQno) {
      case 0:  //Backoutstraight
//         mov_SetTargetSpeed(Speed00,Speed00,300);
         break;
      case 1: //Backout with turn
//         mov_SetTargetSpeed(-Speed30,Speed30,2000);
         break;
      case 2: // Let it turn.
//         SetDuration(2000);
         break;
      case 3: // go out from collision
//         mov_SetTargetSpeed(Speed50,Speed50,1500);
         break;
      default:
//         mov_ContinousCutting(2000);  // Set mowerState to Cutting
      ;
   };
   seQno++;

   return true;
};

bool mBWFBackout::OnEnable() { 
    seQno = 0;

    return true; 
};
void mBWFBackout::OnDisable() { 
  //ForwardCut;
};

