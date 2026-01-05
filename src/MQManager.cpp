// #include "MQManager.hpp"

// // ===== Arduino Mega Analog Pins =====
// static constexpr uint8_t MQ4_PIN   = A6;
// static constexpr uint8_t MQ6_PIN   = A4;
// static constexpr uint8_t MQ8_PIN   = A0;
// static constexpr uint8_t MQ135_PIN = A2;

// // Constructor initialization
// MQManager::MQManager()
//   : mq4(MQ4_PIN),
//     mq6(MQ6_PIN),
//     mq8(MQ8_PIN),
//     mq135(MQ135_PIN)
// {}

// void MQManager::begin() {
//   mq4.begin();
//   mq6.begin();
//   mq8.begin();
//   mq135.begin();
// }

// void MQManager::readOne(const char *name, MQSensor &sensor, Stream &out) {
//   int raw = sensor.readRaw();
//   int avg = sensor.readAvg();
//   float rs  = sensor.computeRsKohm(avg);
//   float ratio = sensor.rsOverRo(rs);

//   out.print(name);
//   out.print(F(" | Raw: "));
//   out.print(raw);
//   out.print(F(" Avg: "));
//   out.print(avg);
//   out.print(F(" Rs(kΩ): "));
//   out.print(rs, 2);
//   out.print(F(" Rs/Ro: "));
//   out.println(ratio, 3);
// }

// void MQManager::readAndPrint(Stream &out) {
//   out.println(F("---- MQ SENSOR READINGS ----"));
//   readOne("MQ-4  ", mq4, out);
//   readOne("MQ-6  ", mq6, out);
//   readOne("MQ-8  ", mq8, out);
//   readOne("MQ-135", mq135, out);
//   out.println(F("----------------------------"));
// }



// #include "MQManager.hpp"

// // ===== Arduino Mega Analog Pins =====
// static constexpr uint8_t MQ4_PIN   = A6;
// static constexpr uint8_t MQ6_PIN   = A4;
// static constexpr uint8_t MQ8_PIN   = A0;
// static constexpr uint8_t MQ135_PIN = A2;

// MQManager::MQManager()
//   : mq4(MQ4_PIN),
//     mq6(MQ6_PIN),
//     mq8(MQ8_PIN),
//     mq135(MQ135_PIN)
// {}

// void MQManager::begin()
// {
//   mq4.begin();
//   mq6.begin();
//   mq8.begin();
//   mq135.begin();
// }

// void MQManager::setRoValues(float ro4, float ro6, float ro8, float ro135)
// {
//   mq4.setRo(ro4);
//   mq6.setRo(ro6);
//   mq8.setRo(ro8);
//   mq135.setRo(ro135);
// }

// void MQManager::readOne(const char *name, MQSensor &sensor, Stream &out)
// {
//   int raw = sensor.readRaw();
//   int avg = sensor.readAvg();
//   float rs = sensor.computeRsKohm(avg);
//   float ratio = sensor.rsOverRo(rs);

//   out.print(name);
//   out.print(F(" | Raw: "));
//   out.print(raw);
//   out.print(F(" Avg: "));
//   out.print(avg);
//   out.print(F(" Rs(kΩ): "));
//   out.print(rs, 2);
//   out.print(F(" Rs/Ro: "));
//   out.println(ratio, 3);
// }

// void MQManager::readAndPrint(Stream &out)
// {
//   out.println(F("---- MQ SENSOR READINGS ----"));
//   readOne("MQ-4  ", mq4, out);
//   readOne("MQ-6  ", mq6, out);
//   readOne("MQ-8  ", mq8, out);
//   readOne("MQ-135", mq135, out);
//   out.println(F("----------------------------"));
// }



//________________________________________________________________________________________________
// #include "MQManager.hpp"
// #include <math.h>

// // ===== Arduino Mega Analog Pins =====
// static constexpr uint8_t MQ4_PIN   = A6;
// static constexpr uint8_t MQ6_PIN   = A4;
// static constexpr uint8_t MQ8_PIN   = A0;
// static constexpr uint8_t MQ135_PIN = A2;

// // ===== Datasheet curve constants =====
// // (Approximate, widely used)
// #define MQ4_CH4_M    -0.38
// #define MQ4_CH4_B     1.34

// #define MQ6_LPG_M    -0.47
// #define MQ6_LPG_B     1.24

// #define MQ8_H2_M     -0.48
// #define MQ8_H2_B      0.90

// #define MQ135_AIR_M  -0.32
// #define MQ135_AIR_B   1.20

// MQManager::MQManager()
//   : mq4(MQ4_PIN),
//     mq6(MQ6_PIN),
//     mq8(MQ8_PIN),
//     mq135(MQ135_PIN)
// {}

// void MQManager::begin()
// {
//   mq4.begin();
//   mq6.begin();
//   mq8.begin();
//   mq135.begin();

//   // CSV header
//   Serial.println(F("time_ms,site,sensor,rs_kohm,rs_ro,ppm"));
// }

// void MQManager::setRoValues(float ro4, float ro6, float ro8, float ro135)
// {
//   mq4.setRo(ro4);
//   mq6.setRo(ro6);
//   mq8.setRo(ro8);
//   mq135.setRo(ro135);
// }

// void MQManager::readOneCSV(
//   uint32_t timeMs,
//   int site,
//   const char *sensorName,
//   MQSensor &sensor,
//   float m,
//   float b
// ) {
//   int avg = sensor.readAvg();
//   float rs = sensor.computeRsKohm(avg);
//   float ratio = sensor.rsOverRo(rs);
//   float ppm = sensor.calculatePPM(ratio, m, b);

//   Serial.print(timeMs);
//   Serial.print(',');
//   Serial.print(site);
//   Serial.print(',');
//   Serial.print(sensorName);
//   Serial.print(',');
//   Serial.print(rs, 3);
//   Serial.print(',');
//   Serial.print(ratio, 3);
//   Serial.print(',');
//   Serial.println(ppm, 2);
// }

// void MQManager::readAndLogCSV(int siteID)
// {
//   uint32_t t = millis();

//   readOneCSV(t, siteID, "MQ4_CH4",   mq4,   MQ4_CH4_M,    MQ4_CH4_B);
//   readOneCSV(t, siteID, "MQ6_LPG",   mq6,   MQ6_LPG_M,    MQ6_LPG_B);
//   readOneCSV(t, siteID, "MQ8_H2",    mq8,   MQ8_H2_M,     MQ8_H2_B);
//   readOneCSV(t, siteID, "MQ135_AIR", mq135, MQ135_AIR_M, MQ135_AIR_B);
// }



//-----------------------------Last-------------------------------------------------------------------


// 

//-------------latest-------------------------------

#include "MQManager.hpp"

// Mega Analog Pins
#define MQ4_PIN    A4
#define MQ136_PIN  A6
#define MQ8_PIN    A0
#define MQ135_PIN  A2

// Datasheet constants
#define MQ4_M    -0.38
#define MQ4_B     1.34
#define MQ136_M  -0.48
#define MQ136_B   1.25
#define MQ8_M    -0.48
#define MQ8_B     0.90
#define MQ135_M  -0.32
#define MQ135_B   1.20

MQManager::MQManager()
: mq4(MQ4_PIN), mq136(MQ136_PIN), mq8(MQ8_PIN), mq135(MQ135_PIN) {}

void MQManager::begin() {
    mq4.begin();
    mq136.begin();
    mq8.begin();
    mq135.begin();

    Serial.println(F("time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa,alt_m"));
}

void MQManager::calibrateAll() {
    Serial.println(F("Calibrating MQ sensors (clean air)..."));
    mq4.calibrate(4.4);
    mq136.calibrate(3.6);
    mq8.calibrate(1.0);
    mq135.calibrate(3.6);
    Serial.println(F("Calibration done."));
}

void MQManager::readAndLogCSV(
    int siteID,
    float temp,
    float hum,
    float press,
    float alt
) {
    uint32_t t = millis();

    logOne(t, siteID, "MQ4_CH4",  mq4.getPPM(MQ4_M, MQ4_B),   "ppm", temp, hum, press, alt);
    logOne(t, siteID, "MQ136_H2S",mq136.getPPM(MQ136_M, MQ136_B),"ppm", temp, hum, press, alt);
    logOne(t, siteID, "MQ8_H2",   mq8.getPPM(MQ8_M, MQ8_B),   "ppm", temp, hum, press, alt);
    logOne(t, siteID, "MQ135_AIR",mq135.getPPM(MQ135_M, MQ135_B),"ppm", temp, hum, press, alt);
}

void MQManager::logEnvCSV(
    uint32_t timeMs,
    int siteID,
    float temp,
    float hum,
    float press,
    float alt
) {
    logOne(timeMs, siteID, "BME_TEMP", temp,  "C",   temp, hum, press, alt);
    logOne(timeMs, siteID, "BME_HUM",  hum,   "%",   temp, hum, press, alt);
    logOne(timeMs, siteID, "BME_PRESS",press, "hPa", temp, hum, press, alt);
    logOne(timeMs, siteID, "BME_ALT",  alt,   "m",   temp, hum, press, alt);
}

void MQManager::logOne(
    uint32_t timeMs,
    int siteID,
    const char* name,
    float value,
    const char* unit,
    float temp,
    float hum,
    float press,
    float alt
) {
    Serial.print(timeMs); Serial.print(",");
    Serial.print(siteID); Serial.print(",");
    Serial.print(name);   Serial.print(",");
    Serial.print(value, 3); Serial.print(",");
    Serial.print(unit);   Serial.print(",");
    Serial.print(temp, 2); Serial.print(",");
    Serial.print(hum, 2);  Serial.print(",");
    Serial.print(press, 2);Serial.print(",");
    Serial.println(alt, 2);
}

