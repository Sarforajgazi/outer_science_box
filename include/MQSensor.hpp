/**
 * =============================================================================
 * MQ SENSOR - Header File
 * =============================================================================
 * 
 * Generic class for reading MQ-series gas sensors.
 * Supports MQ-2, MQ-3, MQ-4, MQ-5, MQ-6, MQ-7, MQ-8, MQ-9, MQ-135, MQ-136, etc.
 * 
 * Features:
 *   - Raw and averaged ADC readings
 *   - Automatic Rs (sensor resistance) calculation
 *   - Calibration using known clean air ratios
 *   - PPM calculation using datasheet curve constants
 *   - Exponential Moving Average (EMA) smoothing
 *   - Spike rejection for noise filtering
 * 
 * Circuit Diagram:
 *   VCC (5V) ─── [Rs SENSOR] ─┬─ [RL LOAD RESISTOR] ─── GND
 *                             │
 *                        Analog Pin (ADC input)
 * 
 * Usage Example:
 *   MQSensor mq4(A0, 20000.0f);    // Pin A0, 20kΩ load resistor
 *   mq4.begin();
 *   mq4.calibrateFromCleanAirRatio(4.4);  // MQ-4 clean air ratio
 *   float ppm = mq4.readPPMSmoothed(-0.36, 1.10);  // Using datasheet curve
 * 
 * =============================================================================
 */

#pragma once
#include <Arduino.h>

class MQSensor {
public:
    // =========================================================================
    // CONSTRUCTOR
    // =========================================================================
    
    /**
     * Create an MQ sensor instance.
     * 
     * @param pin      Analog pin connected to sensor output
     * @param rl_ohms  Load resistor value in ohms (default: 10kΩ)
     * @param ro_kohm  Known Ro in kΩ, or -1 to calibrate later (default: -1)
     */
    MQSensor(uint8_t pin, float rl_ohms = 10000.0f, float ro_kohm = -1.0f);

    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /** Initialize sensor pin as INPUT. Call in setup(). */
    void begin();

    // =========================================================================
    // RAW READINGS
    // =========================================================================
    
    /** Read single raw ADC value (0-1023) */
    int readRaw() const;
    
    /** Read averaged ADC value for noise reduction */
    int readAvg(int samples = 8, unsigned long delay_ms = 5) const;

    // =========================================================================
    // RESISTANCE CALCULATIONS
    // =========================================================================
    
    /** Convert ADC value to voltage (0-5V) */
    float adcToVoltage(int adc) const;
    
    /** Calculate sensor resistance Rs in kΩ from ADC reading */
    float computeRsKohm(int avgAdc) const;

    /** Measure average Rs in clean air (for calibration) */
    float measureRsKohmInCleanAir(
        int samples = 50,
        unsigned long delayMsBetween = 50
    ) const;

    /** Calculate Rs/Ro ratio (key value for PPM lookup) */
    float rsOverRo(float rs_kohm) const;

    // =========================================================================
    // CALIBRATION
    // =========================================================================
    
    /**
     * Calibrate sensor using known clean air Rs/Ro ratio from datasheet.
     * 
     * Common clean air ratios:
     *   MQ-4:   4.4  (Methane)
     *   MQ-136: 3.6  (H2S)
     *   MQ-8:   70   (Hydrogen)
     *   MQ-135: 3.6  (Air Quality)
     */
    void calibrateFromCleanAirRatio(
        float cleanAirRsRoRatio,
        int samples = 100,
        unsigned long delayMs = 10
    );

    // =========================================================================
    // PPM CALCULATION
    // =========================================================================
    
    /**
     * Calculate PPM from Rs/Ro using log-log curve.
     * Formula: PPM = 10^((log10(Rs/Ro) - b) / m)
     */
    float calculatePPM(float rs_ro, float m, float b) const;
    
    /** Read sensor and return PPM (single reading) */
    float readPPM(float m, float b);
    
    /** Read sensor with EMA smoothing (recommended) */
    float readPPMSmoothed(float m, float b);

    // =========================================================================
    // ACCESSORS
    // =========================================================================
    
    /** Get sensor pin number */
    uint8_t pin() const { return _pin; }
    
    /** Get calibrated Ro value in kΩ */
    float roKohm() const { return _ro_kohm; }
    
    /** Manually set Ro value (for pre-calibrated values) */
    void setRoKohm(float ro) { _ro_kohm = ro; }

    // =========================================================================
    // CONSTANTS
    // =========================================================================
    
    static const int ADC_MAX = 1023;           // 10-bit ADC maximum value
    static constexpr float ADC_REF = 5.0f;     // ADC reference voltage (5V)

private:
    // =========================================================================
    // MEMBER VARIABLES
    // =========================================================================
    
    uint8_t _pin;           // Analog pin number
    float _rl_ohms;         // Load resistor value in ohms
    float _ro_kohm;         // Calibrated Ro value in kilo-ohms
    float _ema_ppm = -1.0f; // EMA state (-1 = not initialized)
    
    // =========================================================================
    // SMOOTHING CONSTANTS
    // =========================================================================
    
    // EMA alpha: 0.1 = heavy smoothing (slow response, stable)
    //            0.5 = moderate smoothing
    //            0.9 = light smoothing (fast response, noisy)
    static constexpr float EMA_ALPHA = 0.1f;
    
    // Spike threshold: reject readings > 10x the current average
    // Prevents brief noise from corrupting the smoothed value
    static constexpr float SPIKE_THRESHOLD = 10.0f;
};
