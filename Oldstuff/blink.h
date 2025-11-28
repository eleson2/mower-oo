#ifndef _BLINK_
#define _BLINK_

#define _TASK_OO_CALLBACKS
#include <TaskSchedulerDeclarations.h>


class BlinkTask: public Task  { 
private:
    byte LED = 0;

public:
     BlinkTask(Scheduler* aS,unsigned int mSec);
    ~BlinkTask();
    
    bool Callback();

    bool OnEnable();
    void OnDisable();
};
#endif