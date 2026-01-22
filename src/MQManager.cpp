/**
 * =============================================================================
 * MQ MANAGER - Implementation
 * =============================================================================
 * 
 * This file implements the MQManager class which coordinates all 4 MQ sensors.
 * 
 * Sensors Managed:
 *   - MQ-4:   Methane (CH4) - natural gas, biogas detection
 *   - MQ-136: Hydrogen Sulfide (H2S) - toxic gas, rotting organic matter
 *   - MQ-8:   Hydrogen (H2) - fuel cells, industrial processes
 *   - MQ-135: Air Quality (CO2) - indoor air quality, ventilation
 * 
 * Key Features:
 *   - Auto-calibration using datasheet clean air ratios
 *   - Temperature/humidity compensation for MQ-135
 *   - CSV output format compatible with data logger
 *   - EMA smoothing to reduce noise
 * 
 * =============================================================================
 */

#include "MQManager.hpp"

// =============================================================================
// ANALOG PIN ASSIGNMENTS (Arduino Mega)
// =============================================================================
#define MQ4_PIN    A4    // Methane sensor connected to analog pin A4
#define MQ136_PIN  A6    // H2S sensor connected to analog pin A6
#define MQ8_PIN    A0    // Hydrogen sensor connected to analog pin A0
#define MQ135_PIN  A2    // Air quality sensor connected to analog pin A2

// =============================================================================
// DATASHEET CURVE CONSTANTS
// =============================================================================
// 
// These constants come from the log-log curves in sensor datasheets.
// Formula: log10(Rs/Ro) = m * log10(PPM) + b
// 
// 'm' is the slope (always negative - more gas = lower resistance)
// 'b' is the Y-intercept (adjusted so clean air gives reasonable readings)
//

// MQ-4: Methane (CH4) detection
// Clean air Rs/Ro ratio: ~4.4
// Typical range: 300-10000 ppm
#define MQ4_M    -0.36   // Slope from datasheet curve
#define MQ4_B     1.10   // Intercept (tuned for ~2 ppm in clean air)

// MQ-136: Hydrogen Sulfide (H2S) detection
// Clean air Rs/Ro ratio: ~3.6
// Typical range: 1-100 ppm (very toxic at low concentrations!)
#define MQ136_M  -0.44   // Slope from datasheet curve
#define MQ136_B   0.70   // Intercept (H2S detected at very low ppm)

// MQ-8: Hydrogen (H2) detection
// Clean air Rs/Ro ratio: ~70 (very high!)
// Typical range: 100-10000 ppm
#define MQ8_M    -0.42   // Slope from datasheet curve
#define MQ8_B     1.30   // Intercept (at Rs/Ro=70, ppm approaches 0)

// MQ-135: CO2 / Air Quality detection
// Clean air Rs/Ro ratio: ~3.6
// Typical range: 10-300 ppm (we add 400 baseline for CO2)
#define MQ135_M  -0.42   // Slope from datasheet curve
#define MQ135_B   0.30   // Intercept (adjusted for 400-800 ppm CO2 baseline)

// =============================================================================
// TEMPERATURE & HUMIDITY COMPENSATION
// =============================================================================
// 
// MQ-135 is particularly sensitive to temperature and humidity.
// Readings must be compensated based on environmental conditions.
// Set these values based on your testing location.
//

// ----- KOLKATA (January testing) -----
// Average conditions during January
#define MQ135_TEMP_REF  20.0   // Reference temperature in °C
#define MQ135_HUM_REF   60.0   // Reference humidity in %

// ----- UDUPI, Karnataka (late January) -----
// Uncomment these and comment above for Udupi testing:
// #define MQ135_TEMP_REF  26.0   // Warmer coastal climate
// #define MQ135_HUM_REF   70.0   // Higher humidity

// =============================================================================
// CONSTRUCTOR
// =============================================================================

/**
 * Initialize the MQ Manager with all 4 sensors.
 * Each sensor is created with its analog pin and load resistor value.
 * 
 * Load Resistor (RL) Values:
 *   These are the resistors on your sensor breakout boards.
 *   Common values are 10kΩ, 15kΩ, 20kΩ, 25kΩ.
 *   Check your board or datasheet for exact values.
 */
MQManager::MQManager()
: mq4(MQ4_PIN, 25000.0f),      // MQ-4:   RL = 25 kΩ
  mq136(MQ136_PIN, 20000.0f),  // MQ-136: RL = 20 kΩ
  mq8(MQ8_PIN, 15000.0f),      // MQ-8:   RL = 15 kΩ
  mq135(MQ135_PIN, 15000.0f)   // MQ-135: RL = 15 kΩ
{}

// =============================================================================
// INITIALIZATION
// =============================================================================

/**
 * Initialize all sensors and print CSV header.
 * Call this in setup() after Serial.begin().
 */
void MQManager::begin() {
    // Initialize each sensor's pin as INPUT
    mq4.begin();
    mq136.begin();
    mq8.begin();
    mq135.begin();

    // Print CSV header for data logging
    // This header is parsed by serial_logger.py
    Serial.println(F("time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa"));
}

// =============================================================================
// CALIBRATION
// =============================================================================

/**
 * Calibrate all MQ sensors using clean air as reference.
 * 
 * IMPORTANT: This MUST be done in fresh outdoor air!
 *            Indoor air or contaminated air will give wrong readings.
 * 
 * Each sensor has a known Rs/Ro ratio in clean air (from datasheet).
 * We measure current Rs and calculate Ro accordingly.
 */
void MQManager::calibrateAll() {
    Serial.println(F("Calibrating MQ sensors in clean air..."));

    // MQ-4: Methane sensor
    // Clean air Rs/Ro = 4.4 (from datasheet)
    mq4.calibrateFromCleanAirRatio(4.4);
    Serial.print(F("MQ4 Ro: ")); 
    Serial.print(mq4.roKohm()); 
    Serial.println(F(" kOhm"));

    // MQ-136: H2S sensor
    // Clean air Rs/Ro = 3.6 (from datasheet)
    mq136.calibrateFromCleanAirRatio(3.6);
    Serial.print(F("MQ136 Ro: ")); 
    Serial.print(mq136.roKohm()); 
    Serial.println(F(" kOhm"));

    // MQ-8: Hydrogen sensor
    // Clean air Rs/Ro = 70 (very high ratio - from datasheet)
    mq8.calibrateFromCleanAirRatio(70.0);
    Serial.print(F("MQ8 Ro: ")); 
    Serial.print(mq8.roKohm()); 
    Serial.println(F(" kOhm"));

    // MQ-135: Air quality sensor
    // Clean air Rs/Ro = 3.6 (from datasheet)
    mq135.calibrateFromCleanAirRatio(3.6);
    Serial.print(F("MQ135 raw ADC: ")); 
    Serial.println(analogRead(MQ135_PIN));
    Serial.print(F("MQ135 Ro: ")); 
    Serial.print(mq135.roKohm()); 
    Serial.println(F(" kOhm"));

    Serial.println(F("Calibration complete!"));
}

// =============================================================================
// DATA LOGGING
// =============================================================================

/**
 * Read all MQ sensors and output as CSV lines.
 * Includes environmental data for reference and compensation.
 * 
 * @param siteID  Sample site identifier (change when rover moves)
 * @param temp    Current temperature in °C (from BME280)
 * @param hum     Current humidity in % (from BME280)
 * @param press   Current pressure in hPa (from BME280)
 */
void MQManager::readAndLogCSV(
    int siteID,
    float temp,
    float hum,
    float press
) {
    // Get current timestamp
    uint32_t t = millis();

    // Log MQ-4: Methane (CH4)
    // Uses smoothed reading to reduce noise
    logOne(t, siteID, "MQ4_CH4", 
           mq4.readPPMSmoothed(MQ4_M, MQ4_B), 
           "ppm", temp, hum, press);
    
    // Log MQ-136: Hydrogen Sulfide (H2S)
    logOne(t, siteID, "MQ136_H2S", 
           mq136.readPPMSmoothed(MQ136_M, MQ136_B), 
           "ppm", temp, hum, press);
    
    // Log MQ-8: Hydrogen (H2)
    logOne(t, siteID, "MQ8_H2", 
           mq8.readPPMSmoothed(MQ8_M, MQ8_B), 
           "ppm", temp, hum, press);
    
    // ---------- MQ-135: CO2 with compensation ----------
    // MQ-135 requires temperature/humidity compensation for accuracy
    
    // Get raw reading
    float rawCO2 = mq135.readPPMSmoothed(MQ135_M, MQ135_B);
    
    // Calculate temperature correction
    // Higher temp = lower sensor resistance = artificially high reading
    // We correct this by scaling down when temp is higher than reference
    float tempCorrection = 1.0 + (MQ135_TEMP_REF - temp) * 0.02;  // 2% per °C
    
    // Calculate humidity correction
    // Higher humidity = lower sensor resistance = artificially high reading
    float humCorrection = 1.0 + (MQ135_HUM_REF - hum) * 0.01;     // 1% per % RH
    
    // Combined correction factor
    float correctionFactor = tempCorrection * humCorrection;
    
    // Apply correction and add baseline
    // Outdoor CO2 is typically ~400 ppm, indoor can be 600-1500 ppm
    float compensatedCO2 = rawCO2 * correctionFactor + 400.0;
    
    // Constrain to reasonable range
    compensatedCO2 = constrain(compensatedCO2, 400.0, 5000.0);
    
    // Log compensated CO2 value
    logOne(t, siteID, "MQ135_CO2", compensatedCO2, "ppm", temp, hum, press);
}

/**
 * Log environmental sensor data (from BME280).
 * 
 * @param timeMs  Timestamp in milliseconds
 * @param siteID  Sample site identifier
 * @param temp    Temperature in °C
 * @param hum     Humidity in %
 * @param press   Pressure in hPa
 */
void MQManager::logEnvCSV(
    uint32_t timeMs,
    int siteID,
    float temp,
    float hum,
    float press
) {
    // Log each environmental parameter as a separate CSV line
    logOne(timeMs, siteID, "BME_TEMP",  temp,  "C",   temp, hum, press);
    logOne(timeMs, siteID, "BME_HUM",   hum,   "%",   temp, hum, press);
    logOne(timeMs, siteID, "BME_PRESS", press, "hPa", temp, hum, press);
}

/**
 * Output a single CSV line to Serial.
 * 
 * Format: time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa
 * Example: 123456,1,MQ4_CH4,18.334,ppm,23.95,57.46,1016.12
 * 
 * @param timeMs  Timestamp in milliseconds since startup
 * @param siteID  Sample site identifier
 * @param sensor  Sensor name (e.g., "MQ4_CH4", "BME_TEMP")
 * @param value   Measured value
 * @param unit    Unit of measurement (e.g., "ppm", "C", "hPa")
 * @param temp    Current temperature (for reference)
 * @param hum     Current humidity (for reference)
 * @param press   Current pressure (for reference)
 */
void MQManager::logOne(
    uint32_t timeMs,
    int siteID,
    const char* sensor,
    float value,
    const char* unit,
    float temp,
    float hum,
    float press
) {
    // Output CSV format with comma separators
    Serial.print(timeMs); Serial.print(",");
    Serial.print(siteID); Serial.print(",");
    Serial.print(sensor); Serial.print(",");
    Serial.print(value, 3); Serial.print(",");   // 3 decimal places
    Serial.print(unit); Serial.print(",");
    Serial.print(temp, 2); Serial.print(",");    // 2 decimal places
    Serial.print(hum, 2); Serial.print(",");     // 2 decimal places
    Serial.println(press, 2);                     // 2 decimal places, newline
}
