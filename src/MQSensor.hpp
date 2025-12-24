// #pragma once
// #include <Arduino.h>

// class MQSensor {
// public:
//   explicit MQSensor(uint8_t pin);

//   void begin();

//   int   readRaw();
//   int   readAvg(uint8_t samples = 10);
//   float computeRsKohm(int adc);
//   float rsOverRo(float rs);

//   void  setRo(float ro);
//   float getRo() const;
//   float calculatePPM(float rsRo, float m, float b);


// private:
//   uint8_t _pin;
//   float   _ro;                         // Reference resistance (kΩ)
//   static constexpr float RL_KOHM = 10.0;
// };


#pragma once
#include <Arduino.h>

class MQSensor {
public:
  explicit MQSensor(uint8_t pin);

  void begin();

  int   readRaw();
  int   readAvg(uint8_t samples = 10);
  float computeRsKohm(int adc);
  float rsOverRo(float rs);

  void  setRo(float ro);
  float getRo() const;

  float calculatePPM(float rsRo, float m, float b);

private:
  uint8_t _pin;
  float   _ro;                         // Reference resistance (kΩ)
  static constexpr float RL_KOHM = 10.0;
};
