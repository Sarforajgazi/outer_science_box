// #include "MQSensor.hpp"

// MQSensor::MQSensor(uint8_t pin, float rl_ohms, float ro_kohm)
//     : _pin(pin), _rl_ohms(rl_ohms), _ro_kohm(ro_kohm)
// {
// }

// void MQSensor::begin()
// {
//   pinMode(_pin, INPUT);
// }

// int MQSensor::readRaw() const
// {
//   return analogRead(_pin);
// }

// int MQSensor::readAvg(int samples, unsigned long delayMsBetween) const
// {
//   long sum = 0;
//   for (int i = 0; i < samples; ++i)
//   {
//     sum += analogRead(_pin);
//     if (delayMsBetween)
//       delay(delayMsBetween);
//   }
//   return (int)(sum / samples);
// }

// float MQSensor::adcToVoltage(int adc) const
// {
//   return (adc * ADC_REF) / ADC_MAX;
// }

// float MQSensor::computeRsKohm(int avgAdc) const
// {
//   float vout = adcToVoltage(avgAdc);
//   if (vout <= 0.0f)
//     return -1.0f;
//   float rs = _rl_ohms * (ADC_REF - vout) / vout; // ohms
//   return rs / 1000.0f;                           // convert to kilo-ohm
// }

// float MQSensor::measureRsKohmInCleanAir(int samples, unsigned long delayMsBetween) const
// {
//   long sum = 0;
//   for (int i = 0; i < samples; ++i)
//   {
//     sum += analogRead(_pin);
//     if (delayMsBetween)
//       delay(delayMsBetween);
//   }
//   int avg = (int)(sum / samples);
//   return computeRsKohm(avg);
// }

// float MQSensor::rsOverRo(float rs_kohm) const
// {
//   if (_ro_kohm <= 0.0f)
//     return -1.0f;
//   return rs_kohm / _ro_kohm;
// }

// String MQSensor::toJsonLine(int rawAdc, int avgAdc, float rs_kohm) const
// {
//   float ratio = rsOverRo(rs_kohm);
//   char buf[200];

//   snprintf(buf, sizeof(buf),
//            "{\"t\":%lu,\"pin\":%u,\"adc\":%d,\"avg\":%d,\"rs_kohm\":%.3f,\"rs_ro\":%.3f}",
//            millis(), (unsigned)_pin, rawAdc, avgAdc, (double)rs_kohm, (double)ratio);
//   return String(buf);
// }


// #include "MQSensor.hpp"
// #include <math.h>

// MQSensor::MQSensor(uint8_t pin)
//   : _pin(pin), _ro(-1.0f) {}

// void MQSensor::begin()
// {
//   pinMode(_pin, INPUT);
// }

// int MQSensor::readRaw()
// {
//   return analogRead(_pin);
// }

// int MQSensor::readAvg(uint8_t samples)
// {
//   long sum = 0;
//   for (uint8_t i = 0; i < samples; i++) {
//     sum += analogRead(_pin);
//     delay(5);
//   }
//   return sum / samples;
// }

// float MQSensor::computeRsKohm(int adc)
// {
//   if (adc <= 0) return -1.0;
//   return ((1023.0 - adc) / adc) * RL_KOHM;
// }

// float MQSensor::rsOverRo(float rs)
// {
//   if (_ro <= 0 || rs <= 0) return -1.0;
//   return rs / _ro;
// }

// void MQSensor::setRo(float ro)
// {
//   _ro = ro;
// }

// float MQSensor::getRo() const
// {
//   return _ro;
// }
// float MQSensor::calculatePPM(float rsRo, float m, float b)
// {
//   if (rsRo <= 0) return -1.0;
//   return pow(10, (log10(rsRo) - b) / m);
// }




//------------------Last----------------------------
// #include "MQSensor.hpp"
// #include <math.h>

// MQSensor::MQSensor(uint8_t pin)
//   : _pin(pin), _ro(-1.0f) {}

// void MQSensor::begin()
// {
//   pinMode(_pin, INPUT);
// }

// int MQSensor::readRaw()
// {
//   return analogRead(_pin);
// }

// int MQSensor::readAvg(uint8_t samples)
// {
//   long sum = 0;
//   for (uint8_t i = 0; i < samples; i++) {
//     sum += analogRead(_pin);
//     delay(5);
//   }
//   return sum / samples;
// }

// float MQSensor::computeRsKohm(int adc)
// {
//   if (adc <= 0) return -1.0;
//   return ((1023.0 - adc) / adc) * RL_KOHM;
// }

// float MQSensor::rsOverRo(float rs)
// {
//   if (_ro <= 0 || rs <= 0) return -1.0;
//   return rs / _ro;
// }

// void MQSensor::setRo(float ro)
// {
//   _ro = ro;
// }

// float MQSensor::getRo() const
// {
//   return _ro;
// }

// float MQSensor::calculatePPM(float rsRo, float m, float b)
// {
//   if (rsRo <= 0) return -1.0;
//   return pow(10, (log10(rsRo) - b) / m);
// }


//------------------Last----------------------------


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
