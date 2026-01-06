// #pragma once
// #include <Arduino.h>
// #include "MQSensor.hpp"

// class MQManager {
// public:
//     MQManager();
//     void begin();
//     void readAndPrint(Stream &out = Serial);

// private:
//   MQSensor mq4;
//   MQSensor mq6;
//   MQSensor mq8;
//   MQSensor mq135;

//   void readOne(const char *name, MQSensor &sensor, Stream &out);
// };


// #pragma once
// #include <Arduino.h>
// #include "MQSensor.hpp"

// class MQManager {
// public:
//   MQManager();
//   void begin();
//   void readAndPrint(Stream &out = Serial);

//   // Inject calibrated Ro values
//   void setRoValues(float ro4, float ro6, float ro8, float ro135);

// private:
//   MQSensor mq4;
//   MQSensor mq6;
//   MQSensor mq8;
//   MQSensor mq135;

//   void readOne(const char *name, MQSensor &sensor, Stream &out);
// };

// #pragma once
// #include <Arduino.h>
// #include "MQSensor.hpp"

// class MQManager {
// public:
//   MQManager();
//   void begin();

//   void setRoValues(float ro4, float ro6, float ro8, float ro135);
//   void readAndLogCSV(int siteID);

// private:
//   MQSensor mq4;
//   MQSensor mq6;
//   MQSensor mq8;
//   MQSensor mq135;

//   void readOneCSV(
//     uint32_t timeMs,
//     int site,
//     const char *sensorName,
//     MQSensor &sensor,
//     float m,
//     float b
//   );
// };


//--------------------------------Last--------------------------------------------------

// #pragma once
// #include <Arduino.h>
// #include "MQSensor.hpp"

// class MQManager {
// public:
//   MQManager();
//   void begin();

//   // MQ4, MQ136, MQ8, MQ135
//   void setRoValues(float ro4, float ro136, float ro8, float ro135);
//   void readAndLogCSV(int siteID);

// private:
//   MQSensor mq4;
//   MQSensor mq136;
//   MQSensor mq8;
//   MQSensor mq135;

//   void readOneCSV(
//     uint32_t timeMs,
//     int site,
//     const char *sensorName,
//     MQSensor &sensor,
//     float m,
//     float b
//   );
// };



//-----------------------------latest-------------------------------


#pragma once
#include <Arduino.h>
#include "MQSensor.hpp"

class MQManager {
public:
    MQManager();

    void begin();
    void calibrateAll();

    void readAndLogCSV(
        int siteID,
        float temp,
        float hum,
        float press
    );

    void logEnvCSV(
        uint32_t timeMs,
        int siteID,
        float temp,
        float hum,
        float press
    );

private:
    MQSensor mq4;
    MQSensor mq136;
    MQSensor mq8;
    MQSensor mq135;

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
