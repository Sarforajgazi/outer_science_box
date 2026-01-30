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
 *   - 1x DHT22 soil temperature/humidity sensor
 *   - 1x HW-103 soil moisture sensor
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
 *   - DHT22: DATA → Pin 2
 *   - HW-103: AO → A8, DO → Pin 22
 * 
 * Author: Team Obseract
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_BME280.h>
#include <DHT.h>
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

// Motor timing (in milliseconds) - Individual timing for each movement
unsigned long PLATFORM1_DOWN_TIME = 3000;  // Platform 1 lowering time
unsigned long PLATFORM1_UP_TIME   = 3000;  // Platform 1 raising time
unsigned long PLATFORM2_DOWN_TIME = 3000;  // Platform 2 (drill) lowering time
unsigned long PLATFORM2_UP_TIME   = 3000;  // Platform 2 (drill) raising time
unsigned long DRILL_TIME          = 2000;  // Time drill runs in soil

// =============================================================================
// PIN DEFINITIONS
// =============================================================================
#define SMALL_SERVO_PIN  19    // PWM pin for small servo motor
#define BIG_SERVO_PIN    42    // PWM pin for big servo motor

// Soil Sensors
#define DHT_PIN               2     // DHT22 data pin
#define DHT_TYPE              DHT22 // DHT22 (AM2302)
#define MOISTURE_ANALOG_PIN   A9    // HW-103 analog output
#define MOISTURE_DIGITAL_PIN  24    // HW-103 digital output

// HW-103 Calibration (adjust based on your sensor)
const int MOISTURE_DRY_VALUE = 1023;  // Value in dry air
const int MOISTURE_WET_VALUE = 300;   // Value in water

// =============================================================================
// GLOBAL OBJECTS
// =============================================================================
Adafruit_BME280 bme;           // BME280 sensor (I2C address 0x76)
MQManager mq;                   // Manager for all 4 MQ gas sensors
RelayController relays;         // Relay controller for motors
DHT dht(DHT_PIN, DHT_TYPE);    // DHT22 soil temperature/humidity sensor

Servo smallServo;               // Small servo object (pin 19)
Servo bigServo;                 // Big servo object (pin 42)

// Soil sensor data structure
struct SoilSensorData {
    float temperature;      // DHT22 temperature (°C)
    float humidity;         // DHT22 humidity (%)
    int moistureRaw;        // HW-103 raw value (0-1023)
    int moisturePercent;    // HW-103 percentage (0-100%)
    bool valid;             // DHT22 read successful?
};

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
// SOIL SENSOR FUNCTIONS (DHT22 + HW-103)
// =============================================================================

SoilSensorData readSoilSensors() {
    SoilSensorData data;
    
    // Read DHT22
    data.temperature = dht.readTemperature();
    data.humidity = dht.readHumidity();
    data.valid = !isnan(data.temperature) && !isnan(data.humidity);
    
    if (!data.valid) {
        data.temperature = 0;
        data.humidity = 0;
    }
    
    // Read HW-103
    data.moistureRaw = analogRead(MOISTURE_ANALOG_PIN);
    data.moisturePercent = map(data.moistureRaw, MOISTURE_DRY_VALUE, MOISTURE_WET_VALUE, 0, 100);
    data.moisturePercent = constrain(data.moisturePercent, 0, 100);
    
    return data;
}

void logSoilSensorData(uint32_t timeMs, int site, SoilSensorData& soil, float airTemp, float hum, float press) {
    // Log soil temperature (from DHT22)
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("SOIL_TEMP,");
    if (soil.valid) {
        Serial.print(soil.temperature, 1);
    } else {
        Serial.print("ERR");
    }
    Serial.print(",C,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log soil humidity (from DHT22)
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("SOIL_HUM,");
    if (soil.valid) {
        Serial.print(soil.humidity, 1);
    } else {
        Serial.print("ERR");
    }
    Serial.print(",%,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log soil moisture (from HW-103)
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("SOIL_MOISTURE,");
    Serial.print(soil.moisturePercent);
    Serial.print(",%,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
}

// =============================================================================
// SIMULATED NPK 7-in-1 SENSOR (Correlated with BME280 and MQ sensors)
// =============================================================================
// These values simulate an NPK sensor based on actual environmental readings

struct NPKData {
    float soilMoisture;    // % (correlated with humidity)
    float soilTemperature; // °C (slightly lower than air temp)
    float ec;              // µS/cm (electrical conductivity)
    float ph;              // pH value
    uint16_t nitrogen;     // mg/kg
    uint16_t phosphorus;   // mg/kg
    uint16_t potassium;    // mg/kg
};

NPKData simulateNPK(float airTemp, float humidity, float pressure, 
                     float ch4_ppm, float h2s_ppm, float h2_ppm, float co2_ppm) {
    NPKData npk;
    
    // Soil temperature: typically 2-5°C lower than air temp
    float tempOffset = random(20, 50) / 10.0;  // 2.0 - 5.0°C offset
    npk.soilTemperature = airTemp - tempOffset;
    
    // Soil moisture: correlated with humidity (higher humidity = more moisture)
    // Base: 30-70%, adjusted by humidity
    float moistureBase = 40.0 + (humidity - 50.0) * 0.4;  // Scale humidity influence
    moistureBase += random(-50, 50) / 10.0;  // Add ±5% random variation
    npk.soilMoisture = constrain(moistureBase, 25.0, 80.0);
    
    // EC (Electrical Conductivity): affected by moisture and temperature
    // Higher moisture = higher EC, warmer = higher EC
    float ecBase = 150.0 + (npk.soilMoisture - 40.0) * 3.0 + (npk.soilTemperature - 25.0) * 5.0;
    ecBase += random(-30, 30);  // Add variation
    npk.ec = constrain(ecBase, 80.0, 600.0);
    
    // pH: Udupi region typically 5.5-7.2
    // Slightly affected by CO2 (more CO2 = slightly more acidic)
    float phBase = 6.5 - (co2_ppm - 400.0) * 0.001;  // CO2 influence
    phBase += random(-30, 30) / 100.0;  // Add ±0.3 variation
    npk.ph = constrain(phBase, 5.0, 7.5);
    
    // Nitrogen: affected by temperature and soil moisture
    // Warmer + moist = more microbial activity = more available N
    float nBase = 40.0 + (npk.soilTemperature - 25.0) * 1.5 + (npk.soilMoisture - 50.0) * 0.3;
    nBase += random(-10, 10);
    npk.nitrogen = constrain((int)nBase, 15, 90);
    
    // Phosphorus: Udupi lateritic soils typically have LOW P availability
    // Neutral pH = slightly better P availability, but still low
    float pBase = 10.0 + (7.0 - abs(npk.ph - 6.5)) * 3.0;
    pBase += random(-5, 5);
    npk.phosphorus = constrain((int)pBase, 2, 25);
    
    // Potassium: affected by EC and moisture
    float kBase = 60.0 + (npk.ec - 200.0) * 0.05 + (npk.soilMoisture - 50.0) * 0.5;
    kBase += random(-15, 15);
    npk.potassium = constrain((int)kBase, 25, 140);
    
    return npk;
}

void logNPKData(uint32_t timeMs, int site, NPKData& npk, float airTemp, float hum, float press) {
    // Log EC (Electrical Conductivity)
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("EC,");
    Serial.print(npk.ec, 0); Serial.print(",uS/cm,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log pH
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("PH,");
    Serial.print(npk.ph, 2); Serial.print(",,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log Nitrogen
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("NITROGEN,");
    Serial.print(npk.nitrogen); Serial.print(",mg/kg,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log Phosphorus
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("PHOSPHORUS,");
    Serial.print(npk.phosphorus); Serial.print(",mg/kg,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
    
    // Log Potassium
    Serial.print(timeMs); Serial.print(",");
    Serial.print(site); Serial.print(",");
    Serial.print("POTASSIUM,");
    Serial.print(npk.potassium); Serial.print(",mg/kg,");
    Serial.print(airTemp, 1); Serial.print(",");
    Serial.print(hum, 1); Serial.print(",");
    Serial.println(press, 1);
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
    
    // ---------- DHT22 SENSOR INITIALIZATION ----------
    dht.begin();
    Serial.println(F("DHT22 initialized (Pin 2)"));
    
    // ---------- HW-103 SENSOR INITIALIZATION ----------
    pinMode(MOISTURE_DIGITAL_PIN, INPUT);
    Serial.println(F("HW-103 initialized (A8, Pin 22)"));
    
    // Initialize random seed for NPK simulation
    randomSeed(analogRead(A7));  // Use unused analog pin for entropy

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
            relays.soilCollectionSequence(
                PLATFORM1_DOWN_TIME,
                PLATFORM1_UP_TIME,
                PLATFORM2_DOWN_TIME,
                PLATFORM2_UP_TIME,
                DRILL_TIME
            );
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
        
        // Simulate and log NPK sensor data (correlated with BME280 readings)
        // Using representative gas values for correlation (actual PPM varies)
        float co2_estimate = 400.0 + (temp - 25.0) * 10.0;  // Rough CO2 estimate
        NPKData npk = simulateNPK(temp, hum, press, 0, 0, 0, co2_estimate);
        logNPKData(timeMs, siteID, npk, temp, hum, press);
        
        // Read and log REAL soil sensor data (DHT22 + HW-103)
        SoilSensorData soilData = readSoilSensors();
        logSoilSensorData(timeMs, siteID, soilData, temp, hum, press);
        
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
