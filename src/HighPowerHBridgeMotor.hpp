#ifndef HIGHPOWERHBRIDGEMOTOR_HPP
#define HIGHPOWERHBRIDGEMOTOR_HPP

#include "motor.hpp"
#include "Arduino.h"

/**
 * Driver for 60A High Power MOS Dual Channel H-bridge DC Motor Driver Module
 *
 * Hardware specifications:
 * - Max current: 60A per channel
 * - Supply voltage: 12-30V DC
 * - PWM frequency: up to 60kHz (configured for ~31kHz)
 * - Logic compatible: 3.3V and 5V
 *
 * Control interface:
 * - PA: PWM input for speed control
 * - A1, A2: Direction control pins
 *   - A1=0, A2=0: Brake
 *   - A1=1, A2=0: Forward
 *   - A1=0, A2=1: Reverse
 *   - A1=1, A2=1: Brake (invalid state, treated as brake)
 */
class HighPowerHBridgeMotor : public Motor {
public:
    /**
     * Constructor
     * @param pinPWM - PWM pin connected to PA (must support hardware PWM)
     * @param pinA1  - Digital pin connected to A1 (direction control)
     * @param pinA2  - Digital pin connected to A2 (direction control)
     */
    HighPowerHBridgeMotor(byte pinPWM, byte pinA1, byte pinA2);

    /**
     * Destructor - ensures motor is stopped
     */
    ~HighPowerHBridgeMotor();

    /**
     * Set motor speed and direction
     * @param speed - Speed value from -1023 to +1023 (standardized range)
     *                Positive values = forward rotation (A1=1, A2=0)
     *                Negative values = reverse rotation (A1=0, A2=1)
     *                Zero = brake (A1=0, A2=0)
     *                Range is automatically mapped to 0-255 PWM internally via bit-shift
     */
    void move(int speed) override;

    /**
     * Stop the motor (brake mode)
     * Sets A1=0, A2=0 for active braking
     */
    void stop() override;

    /**
     * Reset motor to initial state
     * Stops motor and resets internal state
     */
    void reset() override;

    /**
     * Get current motor speed
     * @return Current speed (-1023 to +1023)
     */
    int getSpeed() override;

private:
    byte _pinPWM;         // PWM pin (PA)
    byte _pinA1;          // Direction pin 1
    byte _pinA2;          // Direction pin 2
    int _currentSpeed;    // Current speed setting (-1023 to +1023)

    /**
     * Configure hardware PWM for high frequency operation
     * Sets up Timer2 (pins 3, 11) or Timer1 (pins 9, 10) for ~31kHz PWM
     */
    void setupHighFrequencyPWM();

    /**
     * Set direction control pins based on speed
     * @param speed - Positive (forward), negative (reverse), or zero (brake)
     */
    void setDirection(int speed);

    /**
     * Write PWM duty cycle
     * @param dutyCycle - Value from 0 to 255
     */
    void writePWM(uint8_t dutyCycle);
};

#endif // HIGHPOWERHBRIDGEMOTOR_HPP
