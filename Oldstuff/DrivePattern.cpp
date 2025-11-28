#include <Arduino.h>
#include <./patterns/DrivePattern.h>

motorSpeedCallback DrivePattern::setTargetSpeed = NULL;
 
DrivePattern::DrivePattern(unsigned long aInterval , long aIterations, Scheduler* S, bool _enable) 
          : Task(aInterval, aIterations, S, _enable) {
};

DrivePattern::~DrivePattern(){

}; 

void DrivePattern::setCallback(motorSpeedCallback f)  {
   setTargetSpeed = f;
};
