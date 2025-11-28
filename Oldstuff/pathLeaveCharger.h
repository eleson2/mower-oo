#ifndef _PATHLEAVECHARGER_
#define _PATHLEAVECHARGER_

#define _TASK_OO_CALLBACKS

#include <globals.hpp>
#include <./patterns/DrivePattern.h>

class mLeaveCharger: public DrivePattern { 
private:
    byte seQno = 0;

public:
     mLeaveCharger(Scheduler* aS,unsigned int mSec);
    ~mLeaveCharger();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif