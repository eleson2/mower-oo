#ifndef _PATHSTRAIGHT_
#define _PATHSTRAIGHT_

#define _TASK_OO_CALLBACKS

#include <globals.hpp>
#include <./patterns/DrivePattern.h>

class mStraight: public DrivePattern { 
private:
    byte seQno = 0;

public:
     mStraight(Scheduler* aS,unsigned int mSec);
    ~mStraight();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif