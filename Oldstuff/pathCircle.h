#ifndef _PATHCIRCLE_
#define _PATHCIRCLE_

#define _TASK_OO_CALLBACKS

#include <globals.hpp>
#include <./patterns/DrivePattern.h>

class mCircle: public DrivePattern  { 
private:
    byte seQno = 0;

public:
     mCircle(Scheduler* aS,unsigned int mSec);
    ~mCircle();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif