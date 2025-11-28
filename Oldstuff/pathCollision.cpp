#include <Arduino.h>
#include <./patterns/pathCollision.h>
// #include <./patterns/DrivePattern.h> 

 
mCollision::mCollision(Scheduler* aS, unsigned int mSec) 
          : DrivePattern(mSec,TASK_FOREVER,aS,false) {
};

mCollision::~mCollision(){}; 

bool mCollision::Callback() { 

   switch(seQno) {
      case 0:  // Backout within 100 msec
         //setTargetSpeed(10,10,10); // (-Speed30,-Speed30,200);
         break;
      case 1:  // Keep backing out and turn.
//         mov_SetTargetSpeed (-Speed30,-Speed10,1500);
         break;
      case 2: // turn and go forward.
//         mov_SetTargetSpeed (Speed00,Speed40,1400); // Turn
         break;
      case 3:
//         mov_SetTargetSpeed (Speed50,Speed50,2000); // go out from collision
         break;
      default:
//         mov_ContinousCutting(1500);  // Set mowerState to Cutting
   ;
   };
   seQno++;

   return true;
};

bool mCollision::OnEnable() { 
    seQno = 0;

    return true; 
};
void mCollision::OnDisable() { 
  //ForwardCut;
};

