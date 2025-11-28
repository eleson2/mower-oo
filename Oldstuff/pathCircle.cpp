#include <Arduino.h>
#include <./patterns/pathCircle.h> 

 
mCircle::mCircle(Scheduler* aS, unsigned int mSec) 
          : DrivePattern(mSec,TASK_FOREVER,aS,false) {
};

mCircle::~mCircle(){}; 

bool mCircle::Callback() { 

   switch (seQno) {
      case 0:
//         mov_SetTargetSpeed(Speed00,Speed00,800);
         break;
      case 1:
//         mov_SetTargetSpeed(-Speed70,-Speed50,800);
         break;
      case 2:
//         mov_SetTargetSpeed(Speed00,Speed60,900);
         break;
      case 3:
//         mov_SetTargetSpeed(Speed90,MaxSpeedFwd,30000);
         break;
      default:
//         mov_ContinousCutting(1500);
      ;
   };
   seQno++; 
   return true;
};

bool mCircle::OnEnable() { 
    seQno = 0;

    return true; 
};
void mCircle::OnDisable() { 
  //ForwardCut;
};

