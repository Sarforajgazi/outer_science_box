/**
 * =============================================================================
 * OUTER SCIENCE BOX - MAIN CONTROLLER
 * =============================================================================
 * 
 * Project: Team Obseract Rover - Science Mission
 * Board: Arduino Mega 2560
 * 
 * Description:
 *   This is the main controller for the Outer Science Box. It manages:
 *   - 4x MQ gas sensors (CH4, H2S, H2, CO2)
 *   - 1x BME280 environmental sensor (Temperature, Humidity, Pressure)
 *   - 2x Servo motors for mechanical operations
 * 
 * Operation Flow:
 *   1. Initialize all sensors and servos
 *   2. Wait 120 seconds for MQ sensor warmup (required for accurate readings)
 *   3. Auto-calibrate MQ sensors in clean air
 *   4. Enter main loop: sweep servos + log sensor data as CSV
 * 
 * Data Output Format (CSV via Serial @ 9600 baud):
 *   time_ms, site, sensor, value, unit, temp_C, hum_%, press_hPa
 * 
 * Wiring:
 *   - BME280: SDA → D20, SCL → D21 (I2C, address 0x76)
 *   - MQ-4:   A4 (CH4 - Methane)
 *   - MQ-136: A6 (H2S - Hydrogen Sulfide)
 *   - MQ-8:   A0 (H2 - Hydrogen)
 *   - MQ-135: A2 (CO2 - Air Quality)
 *   - Small Servo: Pin 19
 *   - Big Servo: Pin 42
 * 
 * Author: Team Obseract
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_BME280.h>
#include "MQManager.hpp"

// =============================================================================
// PIN DEFINITIONS
// =============================================================================
#define SMALL_SERVO_PIN  19    // PWM pin for small servo motor
#define BIG_SERVO_PIN    42    // PWM pin for big servo motor

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================
Adafruit_BME280 bme;           // BME280 sensor (I2C address 0x76)
MQManager mq;                   // Manager for all 4 MQ gas sensors

Servo smallServo;               // Small servo object (pin 19)
Servo bigServo;                 // Big servo object (pin 42)

int siteID = 1;                 // Current sample site ID (change when rover moves)

// =============================================================================
// SERVO CONTROL FUNCTIONS
// =============================================================================

/**
 * Set the small servo to a specific angle.
 * 
 * @param angle Target angle in degrees (0-180)
 *              Values outside range are constrained automatically
 */
void setSmallServo(int angle) {
    angle = constrain(angle, 0, 180);  // Safety: limit to valid servo range
    smallServo.write(angle);
}

/**
 * Set the big servo to a specific angle.
 * 
 * @param angle Target angle in degrees (0-180)
 *              Values outside range are constrained automatically
 */
void setBigServo(int angle) {
    angle = constrain(angle, 0, 180);  // Safety: limit to valid servo range
    bigServo.write(angle);
}

// =============================================================================
// SETUP - Runs once on power-up/reset
// =============================================================================
void setup() {
    // Initialize serial communication for data logging
    Serial.begin(9600);
    
    // Initialize I2C bus for BME280 sensor
    Wire.begin();

    // ---------- SERVO INITIALIZATION ----------
    // Attach servos to their respective pins
    smallServo.attach(SMALL_SERVO_PIN);
    bigServo.attach(BIG_SERVO_PIN);
    
    // Move both servos to center position (90°)
    smallServo.write(90);
    bigServo.write(90);
    Serial.println(F("Servos initialized (pins 19, 42)"));

    // ---------- BME280 SENSOR INITIALIZATION ----------
    Serial.println(F("Initializing sensors..."));
    
    // Try to connect to BME280 at I2C address 0x76
    // If failed, halt program (sensor is required)
    if (!bme.begin(0x76)) {
        Serial.println(F("BME280 not detected"));
        while (1);  // Stop here if sensor not found
    }
    Serial.println(F("BME280 detected"));
    
    // Initialize MQ sensor manager
    mq.begin();

    // ---------- MQ SENSOR WARMUP ----------
    // MQ sensors require 2+ minutes of heating before readings are stable
    // This is a hardware requirement - cannot be skipped!
    Serial.println(F("Warming MQ sensors (120s)..."));
    for (int i = 120; i > 0; i--) {
        // Print countdown every 10 seconds
        if (i % 10 == 0) {
            Serial.print(F("  ")); 
            Serial.print(i); 
            Serial.println(F("s remaining..."));
        }
        delay(1000);  // Wait 1 second
    }
    Serial.println(F("Warmup complete!"));

    // ---------- MQ SENSOR CALIBRATION ----------
    // Calibrate all MQ sensors using clean air as reference
    // Must be done in fresh outdoor air for accurate readings
    mq.calibrateAll();
}

// =============================================================================
// MAIN LOOP - Runs repeatedly after setup()
// =============================================================================
void loop() {
    // Get current timestamp in milliseconds since startup
    uint32_t timeMs = millis();

    // ---------- SERVO SWEEP (for demonstration/testing) ----------
    // Sweep both servos through their full range
    // Total sweep time: ~2.2 seconds per cycle
    
    // Forward sweep: 0° → 180°
    for (int angle = 0; angle <= 180; angle += 5) {
        setSmallServo(angle);
        setBigServo(180 - angle);  // Big servo moves opposite direction
        delay(30);                  // 30ms between each 5° step
    }
    
    // Reverse sweep: 180° → 0°
    for (int angle = 180; angle >= 0; angle -= 5) {
        setSmallServo(angle);
        setBigServo(180 - angle);  // Big servo moves opposite direction
        delay(30);                  // 30ms between each 5° step
    }

    // ---------- READ ENVIRONMENTAL DATA (BME280) ----------
    float temp  = bme.readTemperature();       // Temperature in °C
    float hum   = bme.readHumidity();          // Relative humidity in %
    float press = bme.readPressure() / 100.0;  // Pressure in hPa (divided by 100 from Pa)

    // ---------- LOG MQ SENSOR DATA ----------
    // Read all 4 MQ sensors and output as CSV
    // Environmental data is included for compensation calculations
    mq.readAndLogCSV(siteID, temp, hum, press);

    // ---------- LOG ENVIRONMENTAL DATA ----------
    // Output BME280 readings as separate CSV lines
    mq.logEnvCSV(timeMs, siteID, temp, hum, press);
    
    // Note: No delay() here - loop timing is controlled by servo sweeps
    // Each loop iteration takes approximately 2.2 seconds
}
