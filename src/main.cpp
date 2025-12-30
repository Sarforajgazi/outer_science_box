// #include <Arduino.h>
// #include "MQManager.hpp"

// MQManager mq;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial && millis() < 2000)
//   {
//     delay(10);
//   }

//   mq.begin();
//   Serial.println(F("MQ Sensor System Ready"));
// }


// void loop()
// {
//   mq.readAndPrint(Serial);
//   delay(2000);
// }



// #include <Arduino.h>
// #include "MQManager.hpp"

// MQManager mq;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial && millis() < 2000) {
//     delay(10);
//   }

//   mq.begin();

//   // ✅ Ro values derived from your clean-air data
//   // mq.setRoValues(
//   //   1.14,   // MQ-4
//   //   1.94,   // MQ-6
//   //   0.024,  // MQ-8
//   //   6.89    // MQ-135
//   // );

//   mq.setRoValues(
//   1.25,   // MQ-4
//   2.04,   // MQ-6
//   0.053,  // MQ-8
//   7.33    // MQ-135
// );


//   Serial.println(F("MQ Sensor System Ready (Calibrated)"));
// }

// void loop()
// {
//   mq.readAndPrint(Serial);
//   delay(2000);
// }


/**/

// #include <Arduino.h>
// #include "MQManager.hpp"

// MQManager mq;

// // Change siteID when rover moves to a new location
// int siteID = 1;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial && millis() < 2000) {
//     delay(10);
//   }

//   mq.begin();

//   /*
//    Ro values chosen so that:
//    Rs/Ro ≈ datasheet clean-air values
//    MQ-4   ≈ 4.4
//    MQ-6   ≈ 10
//    MQ-8   ≈ 70
//    MQ-135 ≈ 3.6
//   */
//   mq.setRoValues(
//     1.25,   // MQ-4
//     2.23,   // MQ-6
//     0.058,  // MQ-8
//     7.86    // MQ-135
//   );

//   Serial.println(F("MQ Sensor System Ready (Datasheet Calibration)"));
// }

// void loop()
// {
//   mq.readAndLogCSV(siteID);
//   delay(2000);
// }




//-------------------------------------------------------------------
// #include <Arduino.h>
// #include <Wire.h>
// #include <Adafruit_BMP280.h>
// #include "MQManager.hpp"

// MQManager mq;
// Adafruit_BMP280 bmp;   // default I2C

// int siteID = 1;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial && millis() < 2000) {
//     delay(10);
//   }

//   // ---------- MQ ----------
//   mq.begin();
//   mq.setRoValues(1.25, 2.23, 0.058, 7.86);

//   // ---------- BMP280 ----------
//   Wire.begin();   // AVR uses fixed I2C pins

//   if (!bmp.begin(0x76)) {   // SDO → GND
//     Serial.println(F("BMP280 not detected!"));
//     while (1);
//   }

//   bmp.setSampling(
//     Adafruit_BMP280::MODE_NORMAL,
//     Adafruit_BMP280::SAMPLING_X2,
//     Adafruit_BMP280::SAMPLING_X16,
//     Adafruit_BMP280::FILTER_X16,
//     Adafruit_BMP280::STANDBY_MS_500
//   );

//   Serial.println(F("MQ + BMP280 Sensor System Ready"));
// }

// void loop()
// {
//   // Ignore everything until 60 seconds have passed
//   if (millis() < 60000) {
//     return; 
//   }
//   unsigned long time_ms = millis();

//   mq.readAndLogCSV(siteID);

//   float temp = bmp.readTemperature();
//   float press = bmp.readPressure() / 100.0;
//   float alt = bmp.readAltitude(1013.25);

//   Serial.print(time_ms);
//   Serial.print(",");
//   Serial.print(siteID);
//   Serial.print(",BMP280_TEMP,");
//   Serial.print(temp, 2);
//   Serial.println(",C");

//   Serial.print(time_ms);
//   Serial.print(",");
//   Serial.print(siteID);
//   Serial.print(",BMP280_PRESS,");
//   Serial.print(press, 2);
//   Serial.println(",hPa");

//   Serial.print(time_ms);
//   Serial.print(",");
//   Serial.print(siteID);
//   Serial.print(",BMP280_ALT,");
//   Serial.print(alt, 2);
//   Serial.println(",m");

//   delay(2000);
// }

//---------------------------------------------------------------------------

// #include <Wire.h>
// #include <Adafruit_BMP280.h>

// Adafruit_BMP280 bmp;

// void setup() {
//   Serial.begin(9600);
//   while (!Serial);

//   Serial.println("BMP280 basic test");

//   Wire.begin();   // Mega: SDA=D20, SCL=D21

//   // Try BOTH possible addresses
//   if (!bmp.begin(0x76)) {
//     Serial.println("0x76 failed, trying 0x77...");
//     if (!bmp.begin(0x77)) {
//       Serial.println("❌ BMP280 not detected at 0x76 or 0x77");
//       while (1);
//     }
//   }

//   Serial.println("✅ BMP280 detected!");

//   bmp.setSampling(
//     Adafruit_BMP280::MODE_NORMAL,
//     Adafruit_BMP280::SAMPLING_X2,
//     Adafruit_BMP280::SAMPLING_X16,
//     Adafruit_BMP280::FILTER_X16,
//     Adafruit_BMP280::STANDBY_MS_500
//   );
// }

// void loop() {
//   Serial.print("Temp = ");
//   Serial.print(bmp.readTemperature());
//   Serial.println(" °C");

//   Serial.print("Pressure = ");
//   Serial.print(bmp.readPressure() / 100.0);
//   Serial.println(" hPa");

//   delay(2000);
// }


//___________________________________________________________________________________

// #include <Arduino.h>
// #include "MQManager.hpp"

// MQManager mq;
// int siteID = 1;

// void setup()
// {
//   Serial.begin(9600);
//   while (!Serial && millis() < 2000) {
//     delay(10);
//   }

//   // ---------- MQ ----------
//   mq.begin();

//   // Set Ro values (update after calibration)
//   mq.setRoValues(
//     1.25,   // MQ4  (CH4)
//     7.86,   // MQ136 (H2S / NH3)
//     0.058,  // MQ8  (H2)
//     3.60    // MQ135 (Air)
//   );

//   Serial.println(F("MQ Sensor System Ready"));
// }

// void loop()
// {
//   // MQ warm-up time
//   if (millis() < 60000) return;

//   mq.readAndLogCSV(siteID);
//   delay(2000);
// }


//----------------------------------------------------updated with bme280 library
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include "MQManager.hpp"

#define SEALEVELPRESSURE_HPA 1013.25

MQManager mq;
Adafruit_BME280 bme;

int siteID = 1;

void setup()
{
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {
    delay(10);
  }

  // ---------- I2C ----------
  // Mega2560 uses:
  // SDA -> D20
  // SCL -> D21
  Wire.begin();

  // ---------- BME280 ----------
  if (!bme.begin(0x76)) {
    Serial.println(F("BME280 not detected!"));
    while (1);
  }

  // ---------- MQ ----------
  mq.begin();

  // Set Ro values (calibrate later)
  mq.setRoValues(
    1.25,   // MQ4   (CH4)
    7.86,   // MQ136 (H2S / NH3)
    0.058,  // MQ8   (H2)
    3.60    // MQ135 (Air)
  );

  Serial.println(F("MQ + BME280 System Ready"));
}

void loop()
{
  // MQ warm-up
  if (millis() < 60000) return;

  uint32_t time_ms = millis();

  // ---------- MQ ----------
  mq.readAndLogCSV(siteID);

  // ---------- BME ----------
  float temp  = bme.readTemperature();           // °C
  float press = bme.readPressure() / 100.0;      // hPa
  float hum   = bme.readHumidity();               // %
  float alt   = bme.readAltitude(SEALEVELPRESSURE_HPA);

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BME_TEMP,");
  Serial.print(temp, 2);
  Serial.println(",C");

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BME_PRESS,");
  Serial.print(press, 2);
  Serial.println(",hPa");

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BME_HUM,");
  Serial.print(hum, 2);
  Serial.println(",%");

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BME_ALT,");
  Serial.print(alt, 2);
  Serial.println(",m");

  delay(2000);
}
