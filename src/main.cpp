/**
 * =============================================================================
 * OUTER SCIENCE BOX - MAIN CONTROLLER (MERGED)
 * =============================================================================
 * 
 * Project: Team Obseract Rover - Science Mission
 * Board: Arduino Mega 2560
 * 
 * Description:
 *   Combined controller for the Outer Science Box:
 *   - 4x MQ gas sensors (CH4, H2S, H2, CO2)
 *   - 1x BME280 environmental sensor (Temperature, Humidity, Pressure)
 *   - 2x Servo motors for mechanical operations
 *   - 8-Channel Relay module for motor control (soil collection)
 * 
 * Operation Flow:
 *   1. Initialize all sensors, servos, and relays
 *   2. Wait 120 seconds for MQ sensor warmup
 *   3. Auto-calibrate MQ sensors in clean air
 *   4. Run soil collection sequence (if ENABLE_MOTORS = true)
 *   5. Log sensor data for LOGGING_DURATION_MS
 *   6. Mission complete
 * 
 * Safety Feature:
 *   Set ENABLE_MOTORS = false to skip motor sequence (sensors still work)
 * 
 * Wiring:
 *   - BME280: SDA → D20, SCL → D21 (I2C, address 0x76)
 *   - MQ-4:   A4 (CH4 - Methane)
 *   - MQ-136: A6 (H2S - Hydrogen Sulfide)
 *   - MQ-8:   A0 (H2 - Hydrogen)
 *   - MQ-135: A2 (CO2 - Air Quality)
 *   - Small Servo: Pin 19
 *   - Big Servo: Pin 42
 *   - Relays: Pins 46,44,45,43,41,39,37,35
 * 
 * Author: Team Obseract
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_BME280.h>
#include "MQManager.hpp"
#include "RelayController.hpp"

// =============================================================================
// CONFIGURATION - CHANGE THESE SETTINGS
// =============================================================================

// SAFETY SWITCH: Set to false to skip motor sequence (sensors still work!)
bool ENABLE_MOTORS = true;

// How long to log sensor data (in milliseconds)
// 5 minutes = 300000 ms
unsigned long LOGGING_DURATION_MS = 300000;

// How often to log sensor readings (in milliseconds)
unsigned long LOG_INTERVAL_MS = 1000;

// Motor timing (in milliseconds)
unsigned long PLATFORM_MOVE_TIME = 3000;  // Time for platform to move
unsigned long DRILL_TIME = 2000;          // Time for drilling

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
RelayController relays;         // Relay controller for motors

Servo smallServo;               // Small servo object (pin 19)
Servo bigServo;                 // Big servo object (pin 42)

int siteID = 1;                 // Current sample site ID

// State tracking
bool soilCollectionDone = false;
unsigned long loggingStartTime = 0;

// =============================================================================
// SERVO CONTROL FUNCTIONS
// =============================================================================

void setSmallServo(int angle) {
    angle = constrain(angle, 0, 180);
    smallServo.write(angle);
}

void setBigServo(int angle) {
    angle = constrain(angle, 0, 180);
    bigServo.write(angle);
}

// =============================================================================
// SETUP - Runs once on power-up/reset
// =============================================================================
void setup() {
    // Initialize serial communication for data logging
    Serial.begin(9600);
    
    // Wait for serial port (max 2 seconds)
    while (!Serial && millis() < 2000) {
        delay(10);
    }
    
    // Print banner
    Serial.println(F(""));
    Serial.println(F("================================================"));
    Serial.println(F("   OUTER SCIENCE BOX - Team Obseract"));
    Serial.println(F("   Merged Controller (Motors + Sensors)"));
    Serial.println(F("================================================"));
    
    // Print configuration
    Serial.print(F("ENABLE_MOTORS: "));
    Serial.println(ENABLE_MOTORS ? F("ON") : F("OFF (sensors only)"));
    Serial.print(F("Logging duration: "));
    Serial.print(LOGGING_DURATION_MS / 1000);
    Serial.println(F(" seconds"));
    Serial.println(F(""));
    
    // Initialize I2C bus for BME280 sensor
    Wire.begin();

    // ---------- RELAY INITIALIZATION ----------
    relays.begin();

    // ---------- SERVO INITIALIZATION ----------
    smallServo.attach(SMALL_SERVO_PIN);
    bigServo.attach(BIG_SERVO_PIN);
    smallServo.write(90);
    bigServo.write(90);
    Serial.println(F("Servos initialized (pins 19, 42)"));

    // ---------- BME280 SENSOR INITIALIZATION ----------
    Serial.println(F("Initializing sensors..."));
    
    if (!bme.begin(0x76)) {
        Serial.println(F("WARNING: BME280 not detected!"));
        // Don't halt - continue without BME if missing
    } else {
        Serial.println(F("BME280 detected"));
    }
    
    // Initialize MQ sensor manager
    mq.begin();

    // ---------- MQ SENSOR WARMUP ----------
    Serial.println(F("Warming MQ sensors (120s)..."));
    for (int i = 120; i > 0; i--) {
        if (i % 10 == 0) {
            Serial.print(F("  ")); 
            Serial.print(i); 
            Serial.println(F("s remaining..."));
        }
        delay(1000);
    }
    Serial.println(F("Warmup complete!"));

    // ---------- MQ SENSOR CALIBRATION ----------
    mq.calibrateAll();
    
    Serial.println(F(""));
    Serial.println(F("================================================"));
    Serial.println(F("   SETUP COMPLETE - Starting mission..."));
    Serial.println(F("================================================"));
    Serial.println(F(""));
}

// =============================================================================
// MAIN LOOP - Runs repeatedly after setup()
// =============================================================================
void loop() {
    // =========================================================================
    // STEP 1: SOIL COLLECTION (runs once, if motors enabled)
    // =========================================================================
    if (!soilCollectionDone) {
        if (ENABLE_MOTORS) {
            Serial.println(F("Running soil collection sequence..."));
            relays.soilCollectionSequence(PLATFORM_MOVE_TIME, DRILL_TIME);
        } else {
            Serial.println(F("Motors disabled - skipping soil collection"));
        }
        
        soilCollectionDone = true;
        loggingStartTime = millis();
        
        Serial.println(F(""));
        Serial.println(F("Starting sensor logging..."));
        Serial.println(F("time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa"));
    }
    
    // =========================================================================
    // STEP 2: SENSOR LOGGING (runs for LOGGING_DURATION_MS)
    // =========================================================================
    unsigned long elapsed = millis() - loggingStartTime;
    
    if (elapsed < LOGGING_DURATION_MS) {
        // Get current timestamp
        uint32_t timeMs = millis();
        
        // Read environmental data from BME280
        float temp  = bme.readTemperature();
        float hum   = bme.readHumidity();
        float press = bme.readPressure() / 100.0;
        
        // Log MQ sensor data as CSV
        mq.readAndLogCSV(siteID, temp, hum, press);
        
        // Log environmental data as CSV
        mq.logEnvCSV(timeMs, siteID, temp, hum, press);
        
        // Print progress every 30 seconds
        if ((elapsed / 1000) % 30 == 0 && elapsed > 0) {
            Serial.print(F("# Logging... "));
            Serial.print(elapsed / 1000);
            Serial.print(F("/"));
            Serial.print(LOGGING_DURATION_MS / 1000);
            Serial.println(F(" seconds"));
        }
        
        // Wait for next reading
        delay(LOG_INTERVAL_MS);
        
    } else {
        // =========================================================================
        // STEP 3: MISSION COMPLETE
        // =========================================================================
        Serial.println(F(""));
        Serial.println(F("================================================"));
        Serial.println(F("   SITE MISSION COMPLETE!"));
        Serial.println(F("================================================"));
        Serial.print(F("Site "));
        Serial.print(siteID);
        Serial.println(F(" data collection finished."));
        Serial.println(F("Arduino halted. Reset to restart."));
        
        // Turn off all relays for safety
        relays.allOff();
        
        // Halt - mission complete
        while (1) {
            delay(1000);
        }
    }
}
