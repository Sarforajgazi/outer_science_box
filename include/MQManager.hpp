/**
 * =============================================================================
 * MQ MANAGER - Header File
 * =============================================================================
 * 
 * Manages all 4 MQ gas sensors for the Outer Science Box.
 * Provides unified interface for calibration, reading, and CSV logging.
 * 
 * Sensors Managed:
 *   ┌──────────┬────────────┬─────────────────────────────────┐
 *   │ Sensor   │ Target Gas │ Typical Use                     │
 *   ├──────────┼────────────┼─────────────────────────────────┤
 *   │ MQ-4     │ CH4        │ Natural gas, biogas detection   │
 *   │ MQ-136   │ H2S        │ Toxic gas, decay detection      │
 *   │ MQ-8     │ H2         │ Hydrogen detection              │
 *   │ MQ-135   │ CO2        │ Indoor air quality              │
 *   └──────────┴────────────┴─────────────────────────────────┘
 * 
 * CSV Output Format:
 *   time_ms, site, sensor, value, unit, temp_C, hum_%, press_hPa
 * 
 * Example Output:
 *   123456,1,MQ4_CH4,18.334,ppm,23.95,57.46,1016.12
 *   123456,1,MQ136_H2S,2.084,ppm,23.95,57.46,1016.12
 *   123456,1,BME_TEMP,23.95,C,23.95,57.46,1016.12
 * 
 * Usage:
 *   MQManager mq;
 *   mq.begin();           // Initialize sensors
 *   mq.calibrateAll();    // Calibrate in clean air
 *   mq.readAndLogCSV(1, temp, hum, press);  // Log all sensors
 * 
 * =============================================================================
 */

#pragma once
#include <Arduino.h>
#include "MQSensor.hpp"

class MQManager {
public:
    // =========================================================================
    // CONSTRUCTOR
    // =========================================================================
    
    /** Create MQ Manager with all 4 sensors */
    MQManager();

    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /**
     * Initialize all sensors and print CSV header.
     * Call this in setup() after Serial.begin().
     */
    void begin();
    
    /**
     * Calibrate all sensors using clean air.
     * MUST be done outdoors in fresh air after 2-minute warmup!
     */
    void calibrateAll();

    // =========================================================================
    // DATA LOGGING
    // =========================================================================
    
    /**
     * Read all MQ sensors and output as CSV.
     * Includes temperature/humidity compensation for MQ-135.
     * 
     * @param siteID  Sample site identifier
     * @param temp    Temperature in °C (from BME280)
     * @param hum     Humidity in % (from BME280)
     * @param press   Pressure in hPa (from BME280)
     */
    void readAndLogCSV(
        int siteID,
        float temp,
        float hum,
        float press
    );

    /**
     * Log environmental data (BME280) as CSV.
     * 
     * @param timeMs  Timestamp in milliseconds
     * @param siteID  Sample site identifier
     * @param temp    Temperature in °C
     * @param hum     Humidity in %
     * @param press   Pressure in hPa
     */
    void logEnvCSV(
        uint32_t timeMs,
        int siteID,
        float temp,
        float hum,
        float press
    );

private:
    // =========================================================================
    // SENSOR OBJECTS
    // =========================================================================
    
    MQSensor mq4;     // MQ-4:   Methane (CH4) on pin A4
    MQSensor mq136;   // MQ-136: H2S on pin A6
    MQSensor mq8;     // MQ-8:   Hydrogen (H2) on pin A0
    MQSensor mq135;   // MQ-135: Air Quality (CO2) on pin A2

    // =========================================================================
    // HELPER METHODS
    // =========================================================================
    
    /**
     * Output a single CSV line to Serial.
     * 
     * @param timeMs  Timestamp in milliseconds
     * @param siteID  Sample site identifier
     * @param sensor  Sensor name (e.g., "MQ4_CH4")
     * @param value   Measured value
     * @param unit    Unit string (e.g., "ppm", "C")
     * @param temp    Current temperature (for reference)
     * @param hum     Current humidity (for reference)
     * @param press   Current pressure (for reference)
     */
    void logOne(
        uint32_t timeMs,
        int siteID,
        const char* sensor,
        float value,
        const char* unit,
        float temp,
        float hum,
        float press
    );
};
