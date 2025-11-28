#include "L298.h"

L298::L298() {
   _pwmVal = 0;
   _speed = 0;
   _pinIN1 = 0;
   _pinIN2 = 0;
   _pinPwmOut = 0;
};

L298::L298(byte pinPWM, byte pinIN1, byte pinIN2) {
   _pwmVal = 0;
   _speed = 0;
   setPins(pinPWM, pinIN1, pinIN2);
};

void L298::setPins(byte pinPWM, byte pinIN1, byte pinIN2) {
   _pinPwmOut = pinPWM; 
   _pinIN1 = pinIN1;
   _pinIN2 = pinIN2;

   pinMode(_pinPwmOut, OUTPUT);
   pinMode(_pinIN1, OUTPUT);
   pinMode(_pinIN2, OUTPUT);
   digitalWrite(_pinIN1, LOW);
   digitalWrite(_pinIN2, LOW);
};

int L298::getSpeed() {
   return _pwmVal;
};

void L298::move(int S) {
   // Constrain to standardized range: -1023 to +1023
   S = constrain(S, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
   _speed = S;

   if (S == 0 ) {
      stop();
      return;
   };

   if(_speed>=0) {
      //Enable Forward
      digitalWrite(_pinIN1, HIGH);
      digitalWrite(_pinIN2, LOW);
   } else {
      //Enable Backward
      digitalWrite(_pinIN1, LOW);
      digitalWrite(_pinIN2, HIGH);
   };

   if (S<0) S = -S;
   // Map standardized range (0-1023) to PWM range (0-255)
   // Using bit-shift for efficiency: 1023 >> 2 = 255 (approx)
   _pwmVal = S >> 2;
   analogWrite(_pinPwmOut, _pwmVal);
};

void L298::stop() {
   //Disable
   digitalWrite(_pinIN1, LOW);
   digitalWrite(_pinIN2, LOW);
   analogWrite(_pinPwmOut, 255);
};

void L298::reset() {
   stop();
   _speed = 0;
   _pwmVal = 0;
};