#ifndef _PATHCOLLISION_
#define _PATHCOLLISION_

#define _TASK_OO_CALLBACKS

#include <globals.hpp>
#include <./patterns/DrivePattern.h>



class mCollision: public DrivePattern { 
private:
    byte seQno = 0;

public:
     mCollision(Scheduler* aS,unsigned int mSec);
    ~mCollision();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif