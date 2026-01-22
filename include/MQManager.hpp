

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
