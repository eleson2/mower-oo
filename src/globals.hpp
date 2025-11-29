#ifndef GLOBALS_HPP
#define GLOBALS_HPP

#define _TASK_OO_CALLBACKS

#include "Arduino.h"
#include <TaskSchedulerDeclarations.h>
#include <queue.h>
#include "MowerTypes.h"

// Debug output control
// Set to 0 to disable all debug output, 1 to enable
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
  #define DEBUG_PRINT(x) Serial.print(x)
  #define DEBUG_PRINTLN(x) Serial.println(x)
  #define DEBUG_PRINT2(x, y) Serial.print(x); Serial.print(y)
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT2(x, y)
#endif

//Typedefs
typedef enum {
   CONTINUOUS = 0,
   CHARGER_BACKOUT,
   BWF_LEFT,
   BWF_RIGHT,
   CIRCLE,
   TURN_LEFT,
   SLOW_DOWN,
   AVOID_OBSTACLE
} CurrentMotion;

typedef int16_t wheelSpeed;

typedef struct {
   wheelSpeed leftSpeed;
   wheelSpeed rightSpeed;
   int mSec;
} const movement;

// Additional angle conversion macros (radians, kept in globals for Arduino compatibility)
#define RADIANS_TO_ANGLE(rad) ((angle_t)((rad) * 1800 / PI))  // rad * 180/PI * 10
#define ANGLE_TO_RADIANS(ang) ((ang) * PI / 1800)  // ang / 10 * PI/180

typedef void (*motorSpeedCallback)(movement m);

// Constants
constexpr wheelSpeed MaxSpeed = 1000; // 128*((sizeof(wheelSpeed)-1)*8) -10; 


constexpr wheelSpeed Speed90 = .9 * MaxSpeed;
constexpr wheelSpeed Speed80 = .8 * MaxSpeed;
constexpr wheelSpeed Speed70 = .7 * MaxSpeed;
constexpr wheelSpeed Speed60 = .6 * MaxSpeed;
constexpr wheelSpeed Speed50 = .5 * MaxSpeed;
constexpr wheelSpeed Speed40 = .4 * MaxSpeed;
constexpr wheelSpeed Speed30 = .3 * MaxSpeed;
constexpr wheelSpeed Speed20 = .2 * MaxSpeed;
constexpr wheelSpeed Speed10 = .1 * MaxSpeed;
constexpr wheelSpeed Speed00 =  0;

constexpr unsigned int WheelUpdateRate = 64; //How many mSec between speed updates.

//// Pin assignments
//DriveUnit
constexpr unsigned int LEFTENABLE = 5;     // PWM support needed
constexpr unsigned int LEFTIN1 = 8;
constexpr unsigned int LEFTIN2 = 9;

constexpr unsigned int RIGHTENABLE = 6;     // PWM support needed
constexpr unsigned int RIGHTIN1 = 10;
constexpr unsigned int RIGHTIN2 = 11;

//Sonar
constexpr unsigned int SONARTRIG = 4;
constexpr unsigned int SONARECHO = 2;      // Interupt attached.

//Boundary Wire Fence detection
constexpr unsigned int BWFINPUT = 3;       // Interupt attached.
constexpr unsigned int BWFSIDE = 7;         // Interupt attached.

typedef  Queue<unsigned int,4,0>  SonarQueue;  // <int,4,0> 

#endif