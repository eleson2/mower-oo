#include "HighPowerHBridgeMotor.hpp"

HighPowerHBridgeMotor::HighPowerHBridgeMotor(byte pinPWM, byte pinA1, byte pinA2)
    : _pinPWM(pinPWM), _pinA1(pinA1), _pinA2(pinA2), _currentSpeed(0) {

    // Configure direction pins as outputs
    pinMode(_pinA1, OUTPUT);
    pinMode(_pinA2, OUTPUT);

    // Initialize to brake state
    digitalWrite(_pinA1, LOW);
    digitalWrite(_pinA2, LOW);

    // Configure PWM pin
    pinMode(_pinPWM, OUTPUT);

    // Setup high frequency PWM
    setupHighFrequencyPWM();

    // Initialize PWM to 0
    writePWM(0);
}

HighPowerHBridgeMotor::~HighPowerHBridgeMotor() {
    stop();
}

void HighPowerHBridgeMotor::setupHighFrequencyPWM() {
    // Configure high-frequency PWM based on the pin
    // For ATmega328P (Arduino Uno):
    // - Timer 2 (pins 3, 11): Can achieve ~31kHz with prescaler=1
    // - Timer 1 (pins 9, 10): Can achieve ~31kHz with prescaler=1
    // - Timer 0 (pins 5, 6): Not recommended (used for millis/delay)

    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

    if (_pinPWM == 3 || _pinPWM == 11) {
        // Timer 2 configuration for ~31kHz PWM
        // Phase-correct PWM mode, prescaler = 1
        // Frequency = 16MHz / (2 * 255 * 1) = 31.37kHz

        TCCR2A = _BV(WGM20);  // Phase-correct PWM mode
        TCCR2B = _BV(CS20);   // Prescaler = 1 (no prescaling)

        if (_pinPWM == 3) {
            TCCR2A |= _BV(COM2B1);  // Clear OC2B on compare match
        } else if (_pinPWM == 11) {
            TCCR2A |= _BV(COM2A1);  // Clear OC2A on compare match
        }
    }
    else if (_pinPWM == 9 || _pinPWM == 10) {
        // Timer 1 configuration for ~31kHz PWM
        // Phase-correct PWM, 8-bit mode, prescaler = 1
        // Frequency = 16MHz / (2 * 255 * 1) = 31.37kHz

        TCCR1A = _BV(WGM10);  // 8-bit phase-correct PWM
        TCCR1B = _BV(CS10);   // Prescaler = 1

        if (_pinPWM == 9) {
            TCCR1A |= _BV(COM1A1);  // Clear OC1A on compare match
        } else if (_pinPWM == 10) {
            TCCR1A |= _BV(COM1B1);  // Clear OC1B on compare match
        }
    }
    else {
        // Fallback to standard analogWrite (490Hz or 980Hz)
        // This will work but frequency will be lower than optimal
        // Consider using pins 3, 9, 10, or 11 for best performance
    }

    #else
    // For other platforms, use standard analogWrite
    // May need platform-specific code for ESP32, etc.
    #endif
}

void HighPowerHBridgeMotor::writePWM(uint8_t dutyCycle) {
    #if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__)

    // Use direct register writes for high-frequency PWM pins
    if (_pinPWM == 3) {
        OCR2B = dutyCycle;  // Timer 2, channel B
    }
    else if (_pinPWM == 11) {
        OCR2A = dutyCycle;  // Timer 2, channel A
    }
    else if (_pinPWM == 9) {
        OCR1A = dutyCycle;  // Timer 1, channel A
    }
    else if (_pinPWM == 10) {
        OCR1B = dutyCycle;  // Timer 1, channel B
    }
    else {
        // Fallback to standard analogWrite
        analogWrite(_pinPWM, dutyCycle);
    }

    #else
    // For other platforms
    analogWrite(_pinPWM, dutyCycle);
    #endif
}

void HighPowerHBridgeMotor::setDirection(int speed) {
    if (speed > 0) {
        // Forward: A1=1, A2=0
        digitalWrite(_pinA1, HIGH);
        digitalWrite(_pinA2, LOW);
    }
    else if (speed < 0) {
        // Reverse: A1=0, A2=1
        digitalWrite(_pinA1, LOW);
        digitalWrite(_pinA2, HIGH);
    }
    else {
        // Brake: A1=0, A2=0
        digitalWrite(_pinA1, LOW);
        digitalWrite(_pinA2, LOW);
    }
}

void HighPowerHBridgeMotor::move(int speed) {
    // Constrain speed to standardized range: -1023 to +1023
    speed = constrain(speed, MOTOR_SPEED_MIN, MOTOR_SPEED_MAX);

    // Store current speed
    _currentSpeed = speed;

    // Set direction pins
    setDirection(speed);

    // Map standardized range (0-1023) to PWM range (0-255)
    // Using bit-shift for efficiency: 1023 >> 2 = 255 (approx)
    uint8_t dutyCycle = abs(speed) >> 2;
    writePWM(dutyCycle);
}

void HighPowerHBridgeMotor::stop() {
    // Brake mode: A1=0, A2=0, PWM=0
    digitalWrite(_pinA1, LOW);
    digitalWrite(_pinA2, LOW);
    writePWM(0);
    _currentSpeed = 0;
}

void HighPowerHBridgeMotor::reset() {
    stop();
}

int HighPowerHBridgeMotor::getSpeed() {
    return _currentSpeed;
}
