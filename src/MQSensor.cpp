
//--------------------Latest---------------------------


#include "MQSensor.hpp"
#include <math.h>

MQSensor::MQSensor(uint8_t pin, float rl_ohms, float ro_kohm)
: _pin(pin), _rl_ohms(rl_ohms), _ro_kohm(ro_kohm) {}

void MQSensor::begin() {
    pinMode(_pin, INPUT);
}

int MQSensor::readRaw() const {
    return analogRead(_pin);
}

int MQSensor::readAvg(int samples, unsigned long delay_ms) const {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(_pin);
        delay(delay_ms);
    }
    return sum / samples;
}

float MQSensor::adcToVoltage(int adc) const {
    return (adc * ADC_REF) / ADC_MAX;
}

float MQSensor::computeRsKohm(int adc) const {
    if (adc <= 0) return 999.9f;
    if (adc >= ADC_MAX) return 0.01f;
    return ((ADC_MAX - adc) / (float)adc) * (_rl_ohms / 1000.0f);
}

float MQSensor::measureRsKohmInCleanAir(
    int samples,
    unsigned long delayMsBetween
) const {
    float sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += computeRsKohm(readRaw());
        delay(delayMsBetween);
    }
    return sum / samples;
}

float MQSensor::rsOverRo(float rs_kohm) const {
    if (_ro_kohm <= 0 || rs_kohm <= 0) return -1.0f;
    return rs_kohm / _ro_kohm;
}

void MQSensor::calibrateFromCleanAirRatio(
    float cleanAirRsRoRatio,
    int samples,
    unsigned long delayMs
) {
    float rs = measureRsKohmInCleanAir(samples, delayMs);
    if (rs > 0 && cleanAirRsRoRatio > 0) {
        _ro_kohm = rs / cleanAirRsRoRatio;
    }
}

float MQSensor::calculatePPM(float rs_ro, float m, float b) const {
    if (rs_ro <= 0 || m == 0) return 0.0f;
    float log_ppm = (log10(rs_ro) - b) / m;
    return pow(10.0f, log_ppm);
}

float MQSensor::readPPM(float m, float b) {
    if (_ro_kohm <= 0) return 0.0f;

    int avgAdc = readAvg();
    
    // Safety check: detect unpowered/disconnected sensor
    // Floating inputs typically read very low (<10) or very high (>1000)
    if (avgAdc < 10 || avgAdc > 1000) {
        return -1.0f;  // Return -1 to indicate invalid/unpowered sensor
    }
    
    float rs = computeRsKohm(avgAdc);
    float ratio = rsOverRo(rs);

    return calculatePPM(ratio, m, b);
}

float MQSensor::readPPMSmoothed(float m, float b) {
    float rawPPM = readPPM(m, b);
    
    // Skip invalid readings
    if (rawPPM < 0) {
        return _ema_ppm > 0 ? _ema_ppm : 0.0f;  // Return last good value
    }
    
    // Initialize EMA on first valid reading
    if (_ema_ppm < 0) {
        _ema_ppm = rawPPM;
    } else {
        // Spike rejection: ignore readings that are way higher than current average
        if (rawPPM > _ema_ppm * SPIKE_THRESHOLD && _ema_ppm > 0.1f) {
            // Spike detected - return current average, don't update
            return _ema_ppm;
        }
        
        // Exponential moving average: new = alpha * raw + (1-alpha) * old
        _ema_ppm = EMA_ALPHA * rawPPM + (1.0f - EMA_ALPHA) * _ema_ppm;
    }
    
    return _ema_ppm;
}
