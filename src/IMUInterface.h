#ifndef IMUINTERFACE_H
#define IMUINTERFACE_H

#include "globals.hpp"
#include "Arduino.h"
#include <Wire.h>

// ICM-20948 I2C addresses
#define ICM20948_ADDR_AD0_LOW   0x68  // AD0 pin = LOW
#define ICM20948_ADDR_AD0_HIGH  0x69  // AD0 pin = HIGH (default)
#define ICM20948_ADDR           ICM20948_ADDR_AD0_HIGH

// ICM-20948 Register Banks
#define ICM20948_REG_BANK_SEL   0x7F

// Bank 0 Registers
#define ICM20948_WHO_AM_I       0x00
#define ICM20948_PWR_MGMT_1     0x06
#define ICM20948_PWR_MGMT_2     0x07
#define ICM20948_ACCEL_XOUT_H   0x2D
#define ICM20948_GYRO_XOUT_H    0x33

// Bank 2 Registers
#define ICM20948_GYRO_CONFIG_1  0x01
#define ICM20948_ACCEL_CONFIG   0x14

// Magnetometer (AK09916) - accessed via I2C master interface
#define AK09916_I2C_ADDR        0x0C
#define AK09916_WHO_AM_I        0x01
#define AK09916_STATUS_1        0x10
#define AK09916_MAG_XOUT_L      0x11
#define AK09916_CONTROL_2       0x31
#define AK09916_CONTROL_3       0x32

// IMU Interface for ICM-20948
// INTEGER-ONLY MATH - No floats!
// Angles in tenths of degrees (0-3599)
// Time in milliseconds
class IMUInterface {
private:
    angle_t _currentHeading;        // Heading in tenths of degrees (0-3599)
    angle_t _headingOffset;         // Calibration offset
    time_ms_t _lastUpdate;          // Last update time in milliseconds
    bool _initialized;
    bool _magnetometerEnabled;

    // Gyro drift compensation (raw sensor units or millidegrees/sec)
    int16_t _gyroBiasX;
    int16_t _gyroBiasY;
    int16_t _gyroBiasZ;

    // Magnetometer calibration (hard iron offset)
    int16_t _magOffsetX;
    int16_t _magOffsetY;
    int16_t _magOffsetZ;

    // Select register bank (ICM-20948 uses bank switching)
    void selectBank(uint8_t bank) {
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_REG_BANK_SEL);
        Wire.write(bank << 4);
        Wire.endTransmission(true);
    }

public:
    IMUInterface() : _currentHeading(0), _headingOffset(0),
                     _lastUpdate(0), _initialized(false),
                     _magnetometerEnabled(false),
                     _gyroBiasX(0), _gyroBiasY(0), _gyroBiasZ(0),
                     _magOffsetX(0), _magOffsetY(0), _magOffsetZ(0) {}

    // Initialize ICM-20948
    void begin(bool useMagnetometer = true) {
        Wire.begin();
        delay(100);  // Allow sensor to power up

        // Reset device (Bank 0, PWR_MGMT_1, bit 7)
        selectBank(0);
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_PWR_MGMT_1);
        Wire.write(0x80);  // Device reset
        Wire.endTransmission(true);
        delay(100);

        // Wake up sensor (clear sleep bit)
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_PWR_MGMT_1);
        Wire.write(0x01);  // Auto select best clock source
        Wire.endTransmission(true);
        delay(10);

        // Enable accelerometer and gyroscope (Bank 0, PWR_MGMT_2)
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_PWR_MGMT_2);
        Wire.write(0x00);  // Enable all axes
        Wire.endTransmission(true);
        delay(10);

        // Configure gyroscope (Bank 2, GYRO_CONFIG_1)
        // ±250°/s range for precise heading tracking
        selectBank(2);
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_GYRO_CONFIG_1);
        Wire.write(0x01);  // 250 dps, 1.1 kHz ODR
        Wire.endTransmission(true);

        // Configure accelerometer (Bank 2, ACCEL_CONFIG)
        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_ACCEL_CONFIG);
        Wire.write(0x01);  // ±2g range, 1.125 kHz ODR
        Wire.endTransmission(true);

        // Return to Bank 0
        selectBank(0);

        _magnetometerEnabled = useMagnetometer;
        _initialized = true;
        _lastUpdate = millis();

        DEBUG_PRINTLN("ICM-20948 initialized");
    }

    // Calibrate gyro (measure bias when stationary)
    void calibrate(int samples = 200) {
        if (!_initialized) return;

        DEBUG_PRINTLN("Calibrating gyro - keep sensor stationary!");

        int32_t sumX = 0, sumY = 0, sumZ = 0;

        selectBank(0);

        for (int i = 0; i < samples; i++) {
            Wire.beginTransmission(ICM20948_ADDR);
            Wire.write(ICM20948_GYRO_XOUT_H);
            Wire.endTransmission(false);
            Wire.requestFrom((uint8_t)ICM20948_ADDR, (uint8_t)6, (uint8_t)true);

            int16_t gyroX = Wire.read() << 8 | Wire.read();
            int16_t gyroY = Wire.read() << 8 | Wire.read();
            int16_t gyroZ = Wire.read() << 8 | Wire.read();

            sumX += gyroX;
            sumY += gyroY;
            sumZ += gyroZ;

            delay(10);  // 10ms between samples
        }

        _gyroBiasX = sumX / samples;
        _gyroBiasY = sumY / samples;
        _gyroBiasZ = sumZ / samples;

        DEBUG_PRINT("Gyro bias: X=");
        DEBUG_PRINT(_gyroBiasX);
        DEBUG_PRINT(" Y=");
        DEBUG_PRINT(_gyroBiasY);
        DEBUG_PRINT(" Z=");
        DEBUG_PRINTLN(_gyroBiasZ);
    }

    // Calibrate magnetometer (for accurate compass heading)
    void calibrateMagnetometer(int samples = 100) {
        if (!_initialized || !_magnetometerEnabled) return;

        // TODO: Implement magnetometer calibration
        _magOffsetX = 0;
        _magOffsetY = 0;
        _magOffsetZ = 0;
    }

    // Update heading from gyro (call frequently, e.g., 50-100Hz)
    // INTEGER MATH ONLY!
    void update() {
        if (!_initialized) return;

        time_ms_t currentTime = millis();
        time_ms_t deltaTimeMs = currentTime - _lastUpdate;
        _lastUpdate = currentTime;

        // Read gyroscope Z-axis (yaw rate)
        selectBank(0);

        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_GYRO_XOUT_H);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)ICM20948_ADDR, (uint8_t)6, (uint8_t)true);

        int16_t gyroX __attribute__((unused)) = Wire.read() << 8 | Wire.read();
        int16_t gyroY __attribute__((unused)) = Wire.read() << 8 | Wire.read();
        int16_t gyroZ = Wire.read() << 8 | Wire.read();

        // ICM-20948 at ±250°/s: 131 LSB/(°/s)
        // Convert to tenths of degrees per second: gyroZ / 131 * 10 = gyroZ / 13.1
        // To avoid float: gyroZ * 10 / 131
        int16_t gyroRateDeciDegPerSec = ((int32_t)(gyroZ - _gyroBiasZ) * 10) / 131;

        // Integrate: heading += rate * (deltaTime / 1000)
        // In tenths of degrees: heading += (rate_in_tenths/sec) * (ms / 1000)
        //                              = (rate_in_tenths/sec * ms) / 1000
        int32_t headingChange = ((int32_t)gyroRateDeciDegPerSec * deltaTimeMs) / 1000;
        _currentHeading += (angle_t)headingChange;

        // Normalize to 0-3599 range (0.0° to 359.9°)
        while (_currentHeading >= ANGLE_360) _currentHeading -= ANGLE_360;
        while (_currentHeading < 0) _currentHeading += ANGLE_360;
    }

    // Update heading from magnetometer (compass - absolute heading)
    // This corrects gyro drift and provides true north reference
    void updateFromMagnetometer() {
        if (!_initialized || !_magnetometerEnabled) return;

        // Note: AK09916 magnetometer requires I2C master interface setup
        // This is a simplified direct read - full implementation would use
        // the ICM-20948's I2C master to read from AK09916 at 0x0C

        // For now, stub implementation - magnetometer requires additional setup
        // Full implementation would:
        // 1. Configure I2C master in ICM-20948
        // 2. Set up slave 0 to read from AK09916
        // 3. Read magnetometer data registers
        // 4. Calculate heading with tilt compensation using accelerometer

        // Placeholder: Use gyro-based heading
        // In production, implement proper mag reading or use library like SparkFun ICM-20948
    }

    // Get current heading in tenths of degrees (0-3599)
    angle_t getHeading() const {
        return _currentHeading;
    }

    // Get current heading in degrees (0-359)
    int getHeadingDegrees() const {
        return ANGLE_TO_DEGREES(_currentHeading);
    }

    // Set heading manually (for calibration or GPS-based correction)
    // Input in tenths of degrees
    void setHeading(angle_t heading) {
        _currentHeading = heading;
        while (_currentHeading >= ANGLE_360) _currentHeading -= ANGLE_360;
        while (_currentHeading < 0) _currentHeading += ANGLE_360;
    }

    // Set heading in degrees
    void setHeadingDegrees(int degrees) {
        setHeading(DEGREES_TO_ANGLE(degrees));
    }

    // Reset heading to zero
    void resetHeading() {
        _currentHeading = 0;
        _headingOffset = 0;
    }

    // Set calibration offset
    void setOffset(angle_t offset) {
        _headingOffset = offset;
    }

    // Check if initialized
    bool isInitialized() const {
        return _initialized;
    }

    // Check if magnetometer is enabled
    bool hasMagnetometer() const {
        return _magnetometerEnabled;
    }

    // Stub: Set heading manually for testing (in tenths of degrees)
    void setHeadingStub(angle_t heading) {
        setHeading(heading);
    }

    // Stub: Set heading in degrees for testing
    void setHeadingStubDegrees(int degrees) {
        setHeadingDegrees(degrees);
    }

    // Get accelerometer data (useful for tilt compensation)
    // Returns acceleration in milli-g's (1000 = 1g)
    void getAcceleration(int16_t& x, int16_t& y, int16_t& z) {
        if (!_initialized) {
            x = y = 0;
            z = 1000;  // Default 1g down
            return;
        }

        selectBank(0);

        Wire.beginTransmission(ICM20948_ADDR);
        Wire.write(ICM20948_ACCEL_XOUT_H);
        Wire.endTransmission(false);
        Wire.requestFrom((uint8_t)ICM20948_ADDR, (uint8_t)6, (uint8_t)true);

        int16_t accelX = Wire.read() << 8 | Wire.read();
        int16_t accelY = Wire.read() << 8 | Wire.read();
        int16_t accelZ = Wire.read() << 8 | Wire.read();

        // ICM-20948 at ±2g: 16384 LSB/g
        // Convert to milli-g's: (raw / 16384.0) * 1000 = raw * 1000 / 16384 ≈ raw / 16
        x = accelX >> 4;  // Approximately /16 for milli-g
        y = accelY >> 4;
        z = accelZ >> 4;
    }

    // Get magnetometer data (raw integer values)
    void getMagnetometer(int16_t& x, int16_t& y, int16_t& z) {
        if (!_magnetometerEnabled) {
            x = y = z = 0;
            return;
        }

        // TODO: Read actual magnetometer
        // Stub values (pointing north)
        x = 0;
        y = 500;  // Arbitrary units
        z = 0;
    }

    // Get gyroscope data (raw angular rates in millidegrees/sec)
    void getGyroscope(int16_t& x, int16_t& y, int16_t& z) {
        // TODO: Read actual gyroscope
        // Stub values
        x = 0;
        y = 0;
        z = 0;
    }
};

#endif
