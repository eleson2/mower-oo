# ICM-20948 Integration Guide

## Overview

The ICM-20948 is a 9-axis motion tracking device that combines:
- **3-axis gyroscope** - Angular velocity (rotation rate)
- **3-axis accelerometer** - Linear acceleration (tilt/orientation)
- **3-axis magnetometer** (AK09916) - Magnetic field (compass heading)

This is superior to the MPU6050 as it includes a magnetometer for absolute heading, eliminating gyro drift over time.

## Hardware Connections

### I2C Wiring (Standard)

```
ICM-20948        Arduino Uno
---------        -----------
VDD         -->  3.3V (NOT 5V! ICM-20948 is 3.3V)
GND         -->  GND
SDA         -->  A4 (SDA)
SCL         -->  A5 (SCL)
AD0         -->  VDD (3.3V) for address 0x69
            -->  GND for address 0x68
```

**Important**: The ICM-20948 is a **3.3V device**. Most breakout boards have voltage regulators and level shifters, but verify your specific module.

### Pin Configuration

In `IMUInterface.h`, the default I2C address is set to **0x69** (AD0=HIGH):
```cpp
#define ICM20948_ADDR_AD0_HIGH  0x69  // AD0 pin = HIGH (default)
```

If you connect AD0 to GND, change to:
```cpp
#define ICM20948_ADDR           ICM20948_ADDR_AD0_LOW  // Use 0x68
```

## Software Integration

### Step 1: Add I2C Library

The ICM-20948 uses Arduino's Wire library for I2C communication. Add to your setup:

```cpp
#include <Wire.h>

void setup() {
    Wire.begin();
    imu.begin(true);  // true = enable magnetometer
}
```

### Step 2: Uncomment Hardware Code

In `IMUInterface.h`, uncomment the I2C communication code blocks:

1. **In `begin()` method** - Uncomment sensor initialization
2. **In `calibrate()` method** - Uncomment gyro calibration
3. **In `update()` method** - Uncomment gyro reading
4. **In `updateFromMagnetometer()` method** - Uncomment magnetometer reading

### Step 3: Update Loop

Call sensor updates frequently:

```cpp
void loop() {
    // Update IMU at ~50-100Hz for smooth heading
    static unsigned long lastIMUUpdate = 0;
    if (millis() - lastIMUUpdate > 10) {  // 100Hz
        imu.update();  // Update from gyro
        lastIMUUpdate = millis();
    }

    // Periodically update from magnetometer to correct drift
    static unsigned long lastMagUpdate = 0;
    if (millis() - lastMagUpdate > 1000) {  // 1Hz
        imu.updateFromMagnetometer();  // Correct heading from compass
        lastMagUpdate = millis();
    }

    // Rest of your code...
}
```

## Register Bank System

The ICM-20948 uses a **register bank system** - registers are organized into 4 banks (0-3). You must select the correct bank before accessing registers.

### Bank Selection

```cpp
void selectBank(uint8_t bank) {
    Wire.beginTransmission(ICM20948_ADDR);
    Wire.write(ICM20948_REG_BANK_SEL);  // 0x7F
    Wire.write(bank << 4);
    Wire.endTransmission(true);
}
```

### Register Bank Organization

- **Bank 0**: General (WHO_AM_I, power management, sensor data)
- **Bank 1**: Self-test, offsets
- **Bank 2**: Gyro/Accel configuration
- **Bank 3**: I2C master interface (for magnetometer)

## Magnetometer Access

The magnetometer (AK09916) is a separate chip accessed through the ICM-20948's I2C master interface. This is more complex than direct I2C access.

### Magnetometer Setup Sequence

1. Enable I2C master mode in ICM-20948
2. Configure I2C master clock
3. Set up slave read/write for AK09916
4. Read magnetometer data through ICM-20948

This is handled in the `begin()` method when `useMagnetometer = true`.

## Calibration

### Gyroscope Calibration

Run when the mower is **stationary**:

```cpp
imu.calibrate(200);  // Average 200 samples to measure bias
```

This measures the gyro's zero-rate offset (drift when not rotating).

### Magnetometer Calibration

The magnetometer needs calibration to compensate for:
- **Hard iron effects**: Permanent magnetic fields (motors, metal)
- **Soft iron effects**: Distortion of Earth's magnetic field

**Calibration procedure**:
1. Call `imu.calibrateMagnetometer()`
2. Slowly rotate the mower through full 360° in all three axes
3. The code records min/max values for each axis
4. Calculates offset: `offset = (max + min) / 2`

**When to calibrate**:
- After installation
- When you change hardware (add/move metal components)
- Periodically (magnetic fields can change)

## Sensor Fusion

For best results, combine gyro and magnetometer:

### Gyro Only (Fast, but drifts)
```cpp
void loop() {
    imu.update();  // Integrates gyro rate to heading
    // Fast response, but accumulates drift over time
}
```

### Magnetometer Only (Slow, but accurate)
```cpp
void loop() {
    imu.updateFromMagnetometer();  // Direct compass heading
    // No drift, but slow and noisy
}
```

### Complementary Filter (Recommended)
```cpp
void loop() {
    // High-frequency gyro for smooth tracking
    if (millis() - lastGyroUpdate > 10) {  // 100Hz
        imu.update();
    }

    // Low-frequency magnetometer for drift correction
    if (millis() - lastMagUpdate > 1000) {  // 1Hz
        imu.updateFromMagnetometer();
    }
}
```

The gyro provides smooth, responsive heading, while the magnetometer periodically corrects drift.

## Coordinate System

ICM-20948 uses **NED convention** (North-East-Down):
- **X-axis**: Forward (North when aligned)
- **Y-axis**: Right (East when aligned)
- **Z-axis**: Down

For lawn mower:
- Mount IMU with **X-axis pointing forward**
- **Z-axis pointing down** (perpendicular to ground)
- Heading calculated from **Z-axis rotation** (yaw)

## Sensitivity Settings

### Gyroscope Range
```cpp
// In IMUInterface.h, GYRO_CONFIG_1 register:
0x00 = ±250°/s   // High precision (recommended for mower)
0x02 = ±500°/s
0x04 = ±1000°/s
0x06 = ±2000°/s
```

For lawn mower, **±250°/s is ideal** - mowers turn slowly, so high precision is better than high range.

### Accelerometer Range
```cpp
// In IMUInterface.h, ACCEL_CONFIG register:
0x00 = ±2g   // Sufficient for mower (recommended)
0x02 = ±4g
0x04 = ±8g
0x06 = ±16g
```

**±2g is sufficient** - mowers don't experience high accelerations.

### Magnetometer Mode
```cpp
// AK09916 has multiple modes:
Single Measurement Mode   // Manual trigger
Continuous Mode 1: 10Hz   // Slow
Continuous Mode 2: 20Hz   // Medium
Continuous Mode 3: 50Hz   // Fast (recommended)
Continuous Mode 4: 100Hz  // Very fast
```

## Troubleshooting

### Problem: Can't communicate with ICM-20948

**Solutions**:
1. Check I2C address (0x68 or 0x69)
2. Verify wiring (SDA/SCL not swapped)
3. Check power (must be 3.3V)
4. Run I2C scanner to detect device:
```cpp
Wire.beginTransmission(ICM20948_ADDR);
byte error = Wire.endTransmission();
if (error == 0) Serial.println("ICM-20948 found!");
```

### Problem: Heading drifts over time

**Solutions**:
1. Calibrate gyro when stationary
2. Enable magnetometer updates
3. Increase magnetometer update rate
4. Implement complementary filter

### Problem: Magnetometer readings erratic

**Solutions**:
1. Calibrate magnetometer (hard iron)
2. Move away from metal objects during calibration
3. Keep ICM-20948 away from motors/wires
4. Shield magnetometer if near strong magnetic fields

### Problem: Noisy readings

**Solutions**:
1. Enable digital low-pass filter (DLPF)
2. Reduce sensor update rate
3. Average multiple readings
4. Add capacitors to power lines (0.1µF + 10µF)

## Example Full Initialization

```cpp
#include <Wire.h>
#include "IMUInterface.h"

IMUInterface imu;

void setup() {
    Serial.begin(115200);
    Wire.begin();

    // Initialize IMU with magnetometer
    imu.begin(true);
    delay(100);

    Serial.println("Calibrating gyro - keep stationary!");
    imu.calibrate(200);
    Serial.println("Gyro calibrated");

    Serial.println("Calibrating magnetometer - rotate slowly in all directions!");
    imu.calibrateMagnetometer(100);
    Serial.println("Magnetometer calibrated");

    Serial.println("IMU ready!");
}

void loop() {
    // Fast gyro updates
    static unsigned long lastGyro = 0;
    if (millis() - lastGyro > 10) {  // 100Hz
        imu.update();
        lastGyro = millis();
    }

    // Slow magnetometer corrections
    static unsigned long lastMag = 0;
    if (millis() - lastMag > 500) {  // 2Hz
        imu.updateFromMagnetometer();
        lastMag = millis();
    }

    // Print heading every second
    static unsigned long lastPrint = 0;
    if (millis() - lastPrint > 1000) {
        Serial.print("Heading: ");
        Serial.print(imu.getHeading());
        Serial.println(" degrees");
        lastPrint = millis();
    }
}
```

## Recommended Arduino Libraries

While the stub code implements basic I2C access, you may prefer a full-featured library:

### Option 1: SparkFun ICM-20948 Library
```
pio lib install "sparkfun/SparkFun 9DoF IMU Breakout - ICM 20948 - Arduino Library"
```

### Option 2: Adafruit ICM20X Library
```
pio lib install "adafruit/Adafruit ICM20X"
```

These libraries handle:
- All register bank switching
- Magnetometer I2C master setup
- Sensor fusion algorithms
- Calibration helpers

To use with your code, wrap the library in the `IMUInterface` class.

## Performance Specifications

### Update Rates
- **Gyro**: Up to 1.125 kHz
- **Accelerometer**: Up to 1.125 kHz
- **Magnetometer**: Up to 100 Hz

### Recommended for Mower
- **Gyro**: 100 Hz (smooth heading tracking)
- **Magnetometer**: 1-10 Hz (drift correction)

### Accuracy
- **Gyro**: ±0.1°/s drift (after calibration)
- **Magnetometer**: ±0.5° heading accuracy (after calibration)

### Power Consumption
- **Active mode**: ~3.5 mA
- **Low-power mode**: ~1.2 mA

## Summary

The ICM-20948 provides **accurate, drift-free heading** for line following:

1. **Gyro**: Fast, smooth heading updates (no lag)
2. **Magnetometer**: Periodic corrections (no drift)
3. **Accelerometer**: Future tilt compensation

For production:
- Uncomment hardware code in `IMUInterface.h`
- Calibrate gyro (stationary)
- Calibrate magnetometer (rotating)
- Use complementary filter (gyro + mag)

The result is **reliable heading tracking** essential for accurate line following.

---

**Hardware**: ICM-20948 9-axis IMU
**Interface**: I2C (Wire library)
**Voltage**: 3.3V
**I2C Address**: 0x69 (default) or 0x68
