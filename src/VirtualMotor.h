#ifndef VIRTUAL_MOTOR_H
#define VIRTUAL_MOTOR_H

#include "motor.hpp"
#include "globals.hpp"

/**
 * VirtualMotor - Motor implementation for testing and debugging
 *
 * Simulates motor behavior without hardware:
 * - Logs all commands to serial output (when DEBUG_ENABLED)
 * - Tracks speed state internally
 * - Use with DriveUnit for hardware-free testing
 */
class VirtualMotor : public Motor {
public:
  VirtualMotor(const char* name = "Virtual") : _speed(0), _name(name) {}

  void move(int speed) override {
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
  int _speed;           // Current speed state (-1023 to +1023)
  const char* _name;    // Motor identifier for debug output
};

#endif
