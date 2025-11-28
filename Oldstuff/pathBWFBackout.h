#ifndef _PATHBWFBACKOUT_
#define _PATHBWFBACKOUT_

#define _TASK_OO_CALLBACKS

#include <globals.hpp>
#include <./patterns/DrivePattern.h>

class mBWFBackout: public DrivePattern  { 
private:
    byte seQno = 0;

public:
     mBWFBackout(Scheduler* aS,unsigned int mSec);
    ~mBWFBackout();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif