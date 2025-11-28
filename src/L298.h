#ifndef L298_h
#define L298_h

#include "Arduino.h"
#include "motor.hpp"

class L298 : public Motor  {
//From motor interface
public:
   // constructor
   L298();
   L298(byte pinEnable, byte pinIN1, byte pinIN2);

   void setPins(byte pinPWM, byte pinIN1, byte pinIN2);

   void move(int Speed) override;
   void stop()          override;
   void reset()         override;
   int  getSpeed()      override;

private:
   byte _pinPwmOut;
   byte _pinIN1;
   byte _pinIN2;
   int   _speed;
   byte _pwmVal;
};
#endif