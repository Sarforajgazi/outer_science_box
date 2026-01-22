
//----------------------------Latest vesrion------------------
 
#include <Arduino.h>
#include <Wire.h>
#include <Servo.h>
#include <Adafruit_BME280.h>
#include "MQManager.hpp"

// ========== SERVO PINS ==========
#define SMALL_SERVO_PIN  19
#define BIG_SERVO_PIN    42

Adafruit_BME280 bme;
MQManager mq;

// ========== SERVO OBJECTS ==========
Servo smallServo;  // Small servo on pin 19
Servo bigServo;    // Big servo on pin 42

int siteID = 1;

// ========== SERVO CONTROL FUNCTIONS ==========
void setSmallServo(int angle) {
    angle = constrain(angle, 0, 180);
    smallServo.write(angle);
}

void setBigServo(int angle) {
    angle = constrain(angle, 0, 180);
    bigServo.write(angle);
}

void setup() {
    Serial.begin(9600);
    Wire.begin();

    // ---------- SERVO INIT ----------
    smallServo.attach(SMALL_SERVO_PIN);
    bigServo.attach(BIG_SERVO_PIN);
    smallServo.write(90);  // Center position
    bigServo.write(90);    // Center position
    Serial.println(F("Servos initialized (pins 19, 42)"));

    Serial.println(F("Initializing sensors..."));
    if (!bme.begin(0x76)) {
        Serial.println(F("BME280 not detected"));
        while (1);
    }
    Serial.println(F("BME280 detected"));
    mq.begin();

    // ---------- MQ Warm-up ----------
    Serial.println(F("Warming MQ sensors (120s)..."));
    for (int i = 120; i > 0; i--) {
        if (i % 10 == 0) {  // Print every 10 seconds
            Serial.print(F("  ")); 
            Serial.print(i); 
            Serial.println(F("s remaining..."));
        }
        delay(1000);
    }
    Serial.println(F("Warmup complete!"));

    // ---------- Auto Calibration ----------
    mq.calibrateAll();
}

void loop() {
    uint32_t timeMs = millis();

    // ----- CONTINUOUS SERVO SWEEP -----
    // Sweep from 0° to 180°
    for (int angle = 0; angle <= 180; angle += 5) {
        setSmallServo(angle);
        setBigServo(180 - angle);  // Opposite direction
        delay(30);
    }
    // Sweep back from 180° to 0°
    for (int angle = 180; angle >= 0; angle -= 5) {
        setSmallServo(angle);
        setBigServo(180 - angle);  // Opposite direction
        delay(30);
    }

    // ----- BME280 -----
    float temp  = bme.readTemperature();             // °C
    float hum   = bme.readHumidity();                // %
    float press = bme.readPressure() / 100.0;        // hPa

    // ----- MQ Sensors -----
    mq.readAndLogCSV(siteID, temp, hum, press);

    // ----- Environment as CSV -----
    mq.logEnvCSV(timeMs, siteID, temp, hum, press);
}


