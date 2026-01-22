
//-------------latest-------------------------------

#include "MQManager.hpp"

// Mega analog pins
#define MQ4_PIN    A4
#define MQ136_PIN  A6
#define MQ8_PIN    A0
#define MQ135_PIN  A2

// Datasheet curve constants (log-log: log10(Rs/Ro) = m * log10(ppm) + b)
// These are calibrated so clean air ratio gives ~0 ppm
// MQ-4: CH4 detection, clean air Rs/Ro = 4.4
#define MQ4_M    -0.36
#define MQ4_B     1.10   // Increased to get ~2 ppm in clean air

// MQ-136: H2S detection, clean air Rs/Ro = 3.6
#define MQ136_M  -0.44
#define MQ136_B   0.70   // Adjusted for H2S (detected at low ppm)

// MQ-8: H2 detection, clean air Rs/Ro = 70
#define MQ8_M    -0.42
#define MQ8_B     1.30   // Adjusted: at Rs/Ro=70, ppm ≈ 0

// MQ-135: CO2 detection (compensated), clean air Rs/Ro = 3.6
#define MQ135_M  -0.42
#define MQ135_B   0.30   // Adjusted for 400-800 ppm CO2 baseline

// Temperature/Humidity compensation constants for MQ-135
// Uncomment the location you're testing in:

// ----- KOLKATA (Jan testing) -----
#define MQ135_TEMP_REF  20.0   // Avg temp in Kolkata Jan
#define MQ135_HUM_REF   60.0   // Avg humidity in Kolkata Jan

// ----- UDUPI, Karnataka (Jan 24 - Feb 3) -----
// #define MQ135_TEMP_REF  26.0   // Avg temp in Udupi late Jan
// #define MQ135_HUM_REF   70.0   // Avg humidity in Udupi

MQManager::MQManager()
: mq4(MQ4_PIN, 25000.0f),      // MQ-4:   RL = 25 kΩ
  mq136(MQ136_PIN, 20000.0f),  // MQ-136: RL = 20 kΩ
  mq8(MQ8_PIN, 15000.0f),      // MQ-8:   RL = 15 kΩ
  mq135(MQ135_PIN, 15000.0f)   // MQ-135: RL = 15 kΩ
{}

void MQManager::begin() {
    mq4.begin();
    mq136.begin();
    mq8.begin();
    mq135.begin();

    Serial.println(F("time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa"));
}

void MQManager::calibrateAll() {
    Serial.println(F("Calibrating MQ sensors in clean air..."));

    mq4.calibrateFromCleanAirRatio(4.4);   // MQ-4 (CH4)
    Serial.print(F("MQ4 Ro: ")); Serial.print(mq4.roKohm()); Serial.println(F(" kOhm"));

    mq136.calibrateFromCleanAirRatio(3.6); // MQ-136 (H2S)
    Serial.print(F("MQ136 Ro: ")); Serial.print(mq136.roKohm()); Serial.println(F(" kOhm"));

    mq8.calibrateFromCleanAirRatio(70.0);  // MQ-8 (H2)
    Serial.print(F("MQ8 Ro: ")); Serial.print(mq8.roKohm()); Serial.println(F(" kOhm"));

    mq135.calibrateFromCleanAirRatio(3.6); // MQ-135 (Air quality)
    Serial.print(F("MQ135 raw ADC: ")); Serial.println(analogRead(MQ135_PIN));
    Serial.print(F("MQ135 Ro: ")); Serial.print(mq135.roKohm()); Serial.println(F(" kOhm"));

    Serial.println(F("Calibration complete!"));
}

void MQManager::readAndLogCSV(
    int siteID,
    float temp,
    float hum,
    float press
) {
    uint32_t t = millis();

    logOne(t, siteID, "MQ4_CH4",   mq4.readPPMSmoothed(MQ4_M, MQ4_B),     "ppm", temp, hum, press);
    logOne(t, siteID, "MQ136_H2S", mq136.readPPMSmoothed(MQ136_M, MQ136_B),"ppm", temp, hum, press);
    logOne(t, siteID, "MQ8_H2",    mq8.readPPMSmoothed(MQ8_M, MQ8_B),     "ppm", temp, hum, press);
    
    // MQ-135 CO2 with temperature/humidity compensation
    float rawCO2 = mq135.readPPMSmoothed(MQ135_M, MQ135_B);
    
    // Correction factor based on environmental conditions
    // Higher temp = lower resistance = artificially high reading (correct down)
    // Higher humidity = lower resistance = artificially high reading (correct down)
    float tempCorrection = 1.0 + (MQ135_TEMP_REF - temp) * 0.02;  // 2% per degree
    float humCorrection = 1.0 + (MQ135_HUM_REF - hum) * 0.01;     // 1% per % RH
    float correctionFactor = tempCorrection * humCorrection;
    
    // Apply correction and scale to realistic CO2 range (400-2000 ppm)
    float compensatedCO2 = rawCO2 * correctionFactor + 400.0;  // Baseline ~400 ppm outdoor
    compensatedCO2 = constrain(compensatedCO2, 400.0, 5000.0); // Limit range
    
    logOne(t, siteID, "MQ135_CO2", compensatedCO2, "ppm", temp, hum, press);
}

void MQManager::logEnvCSV(
    uint32_t timeMs,
    int siteID,
    float temp,
    float hum,
    float press
) {
    logOne(timeMs, siteID, "BME_TEMP",  temp,  "C",   temp, hum, press);
    logOne(timeMs, siteID, "BME_HUM",   hum,   "%",   temp, hum, press);
    logOne(timeMs, siteID, "BME_PRESS", press, "hPa", temp, hum, press);
}

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
    Serial.print(timeMs); Serial.print(",");
    Serial.print(siteID); Serial.print(",");
    Serial.print(sensor); Serial.print(",");
    Serial.print(value, 3); Serial.print(",");
    Serial.print(unit); Serial.print(",");
    Serial.print(temp, 2); Serial.print(",");
    Serial.print(hum, 2); Serial.print(",");
    Serial.println(press, 2);
}
