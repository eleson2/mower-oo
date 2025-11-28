#ifndef MOTOR_HPP
#define MOTOR_HPP

#include "Arduino.h"

/**
 * Motor Interface
 *
 * Abstract base class for all motor driver implementations.
 *
 * IMPORTANT SPECIFICATION:
 * - Speed range: -1023 to +1023 (standardized across all motor drivers)
 *   - Positive values: Forward direction
 *   - Negative values: Reverse direction
 *   - Zero: Stop/brake
 * - Range of 1023 chosen for efficient mapping to 8-bit PWM (255)
 *   - 1023 / 255 = 4, allowing simple right-shift >> 2 operation
 *   - Much faster than division on 8-bit microcontrollers
 * - Each implementation must map this range to its hardware capabilities
 * - This standardization allows consistent control across different motor drivers
 */
class Motor {
   public:
      Motor() {};
      virtual ~Motor() {};

      /**
       * Move the motor at specified speed and direction
       *
       * @param speed Motor speed and direction
       *              Range: -1023 to +1023
       *              - Positive: Forward rotation
       *              - Negative: Reverse rotation
       *              - Zero: Stop (brake mode)
       *
       * Note: Implementations should map this standardized range to their
       *       hardware-specific PWM values (e.g., 0-255 for 8-bit PWM)
       *
       * Example mapping for 8-bit PWM:
       *   abs(speed) >> 2  (efficient bit-shift, since 1023/255 = 4)
       *   or: map(abs(speed), 0, 1023, 0, 255)
       */
      virtual void move(int speed) = 0;

      /**
       * Stop the motor immediately
       *
       * Implementations should use active braking (short motor terminals)
       * rather than coasting for better control and safety.
       */
      virtual void stop() = 0;

      /**
       * Reset motor to initial state
       *
       * Should stop the motor and clear any internal state variables.
       * After reset, motor should be in a safe, stopped state.
       */
      virtual void reset() = 0;

      /**
       * Get current motor speed setting
       *
       * @return Current speed value in range -1023 to +1023
       *         - Positive: Moving forward
       *         - Negative: Moving reverse
       *         - Zero: Stopped
       */
      virtual int getSpeed() = 0;

   protected:
      // Constants for standardized speed range
      // 1023 chosen for efficient bit-shift mapping to 8-bit PWM (1023 >> 2 = 255)
      static const int MOTOR_SPEED_MAX = 1023;
      static const int MOTOR_SPEED_MIN = -1023;
};

#endif