#ifndef _MYTASK_
#define _MYTASK_

#define _TASK_OO_CALLBACKS

#include <Arduino.h>
#include <TaskSchedulerDeclarations.h>
#include <globals.hpp>
#include <driveunit.h>

class DrivePattern: public Task { 

private:
    byte seQno = 0;

protected:
    static motorSpeedCallback setTargetSpeed ;

public:

    DrivePattern(unsigned long aInterval , long aIterations, Scheduler *S, bool enable);
    ~DrivePattern();

     static void setCallback(motorSpeedCallback f);
};

#endif