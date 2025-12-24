#pragma once
#include <Arduino.h>

class MQSensor
{
public:
    MQSensor(uint8_t pin, float rl_ohms = 10000.0f, float ro_kohm = -1.0f);

    void begin();

    // Read one analog sample & averaged reading
    int readRaw() const;
    int readAvg(int samples = 8, unsigned long delay_ms = 5) const;

    // Convert ADC to voltage & compute Rs in kiloOhms
    float adcToVoltage(int adc) const;
    float computeRsKohm(int avgAdc) const;

    // Calibration helper: measure Rs (kOhm) over many samples (assumed clean-air)
    float measureRsKohmInCleanAir(int samples = 50, unsigned long delayMsBetween = 50) const;

    // Return Rs/Ro ratio (-1 if Ro unknown or invalid)
    float rsOverRo(float rs_kohm) const;

    // Return a JSON line describing the reading. Uses millis() timestamp.
    String toJsonLine(int rawAdc, int avgAdc, float rs_kohm) const;

    // Accessors
    uint8_t pin() const { return _pin; }
    float roKohm() const { return _ro_kohm; }
    void setRoKohm(float ro) { _ro_kohm = ro; }

    // Static/common
    static const int ADC_MAX = 1023;
    static constexpr float ADC_REF = 5.0f;

private:
    uint8_t _pin;
    float _rl_ohms;
    float _ro_kohm;
};