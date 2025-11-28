#ifndef VIRTUAL_MOTOR_H
#define VIRTUAL_MOTOR_H

#include "motor.hpp"
#include "globals.hpp"

class VirtualMotor : public Motor {
public:
  VirtualMotor(const char* name = "Virtual") : _speed(0), _name(name) {}
  void move(int speed) override {
    // store and optionally log
    _speed = constrain(speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);
    DEBUG_PRINT2(_name, ": move ");
    DEBUG_PRINTLN(_speed);
  }
  void stop() override {
    _speed = 0;
    DEBUG_PRINT2(_name, ": stop\n");
  }
  void reset() override {
    _speed = 0;
    DEBUG_PRINT2(_name, ": reset\n");
  }
  int getSpeed() override { return _speed; }

private:
  int _speed;
  const char* _name;
};

#endif
