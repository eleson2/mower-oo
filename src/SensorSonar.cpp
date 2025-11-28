#include <Arduino.h>
#include <sensorSonar.h> 
#include <TaskSchedulerDeclarations.h>
#include <YetAnotherPcInt.h>
#include <globals.hpp>

sSonar::sSonar(Scheduler* aS, unsigned int timeOut,  SonarQueue *_Response,
               uint8_t trigger_pin, uint8_t _response_pin) 
            : Task(1,timeOut, aS, false) {

   TriggerPin  = trigger_pin;
   ResponsePin = _response_pin;
   Response = _Response; 

   pinMode(TriggerPin, OUTPUT);
   pinMode(ResponsePin, INPUT);

   _responseStartMicros = 0;
   SonarDistance = 0;
};

sSonar::~sSonar(){ }; 

void sSonar::Measure() {
   SonarDistance = 0; 
	PcInt::detachInterrupt(ResponsePin);

	digitalWrite(TriggerPin, LOW);
	delayMicroseconds(4);
	digitalWrite(TriggerPin, HIGH);
	delayMicroseconds(10);
	digitalWrite(TriggerPin, LOW);
	delayMicroseconds(4);

	PcInt::attachInterrupt(ResponsePin, responseStart, this, RISING);
};

bool sSonar::Callback() { 
   if (SonarDistance == 0) return true; 
   Response->push(SonarDistance);
   Measure();
   return true;
};

bool sSonar::OnEnable() { 
   Measure();
   return true;
};

void sSonar::OnDisable() { 
   restartDelayed(2);
};

void sSonar::Stop() {
   PcInt::detachInterrupt(ResponsePin);
   disable();
};

//
// Static interupt handlers
void sSonar::responseStart(sSonar *_this) {
   PcInt::detachInterrupt(_this->ResponsePin);
   _this->_responseStartMicros = micros();
   PcInt::attachInterrupt(_this->ResponsePin, responseEnd, _this, FALLING);
};

// Static interupt handlers
void sSonar::responseEnd(sSonar *_this) {
   PcInt::detachInterrupt(_this->ResponsePin);
   _this->SonarDistance = micros() - _this->_responseStartMicros;
};

unsigned int sSonar::SonarInMM(unsigned int distance) { 
   return distance * 128 / _soundSpeedFactor; 
};

unsigned int sSonar::MMtoMeasure(unsigned int MM) { 
   return MM *  _soundSpeedFactor / 128 ; 
};
