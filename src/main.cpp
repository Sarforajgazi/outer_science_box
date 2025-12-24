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


#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BMP280.h>
#include "MQManager.hpp"

MQManager mq;
Adafruit_BMP280 bmp;   // default I2C

int siteID = 1;

void setup()
{
  Serial.begin(9600);
  while (!Serial && millis() < 2000) {
    delay(10);
  }

  // ---------- MQ ----------
  mq.begin();
  mq.setRoValues(1.25, 2.23, 0.058, 7.86);

  // ---------- BMP280 ----------
  Wire.begin();   // AVR uses fixed I2C pins

  if (!bmp.begin(0x76)) {   // SDO → GND
    Serial.println(F("BMP280 not detected!"));
    while (1);
  }

  bmp.setSampling(
    Adafruit_BMP280::MODE_NORMAL,
    Adafruit_BMP280::SAMPLING_X2,
    Adafruit_BMP280::SAMPLING_X16,
    Adafruit_BMP280::FILTER_X16,
    Adafruit_BMP280::STANDBY_MS_500
  );

  Serial.println(F("MQ + BMP280 Sensor System Ready"));
}

void loop()
{
  // Ignore everything until 60 seconds have passed
  if (millis() < 60000) {
    return; 
  }
  unsigned long time_ms = millis();

  mq.readAndLogCSV(siteID);

  float temp = bmp.readTemperature();
  float press = bmp.readPressure() / 100.0;
  float alt = bmp.readAltitude(1013.25);

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BMP280_TEMP,");
  Serial.print(temp, 2);
  Serial.println(",C");

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BMP280_PRESS,");
  Serial.print(press, 2);
  Serial.println(",hPa");

  Serial.print(time_ms);
  Serial.print(",");
  Serial.print(siteID);
  Serial.print(",BMP280_ALT,");
  Serial.print(alt, 2);
  Serial.println(",m");

  delay(2000);
}
