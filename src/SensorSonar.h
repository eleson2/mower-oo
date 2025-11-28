#ifndef SENSORSONAR_H
#define SENSORSONAR_H

#define _TASK_OO_CALLBACKS

#include <TaskSchedulerDeclarations.h>
#include <Arduino.h>
#include <YetAnotherPcInt.h> 
#include <globals.hpp>

class sSonar: public Task  { 
private:
   uint8_t TriggerPin;
   uint8_t ResponsePin;
    
   volatile unsigned long _responseStartMicros;

   unsigned int SonarDistance;

   SonarQueue *Response;
   static constexpr int  _soundSpeedFactor =2560000 / (3310 + 6 * 22);

public:
   sSonar(Scheduler* aS, unsigned int timeOut, SonarQueue *_Response,
          uint8_t   trigger_pin, uint8_t _response_pin) ;
   ~sSonar();
    
   bool Callback() override;
   bool OnEnable() override;
   void OnDisable() override;
   void Measure();
   void Stop();

   static void responseStart(sSonar *);
   static void responseEnd(sSonar *);
   static unsigned int SonarInMM(unsigned int distance); 
   static unsigned int MMtoMeasure(unsigned int MM); 
    
   void SonarTimeoutCallback();
};
#endif