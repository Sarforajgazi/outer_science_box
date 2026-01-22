/**
 * =============================================================================
 * MQ SENSOR - Implementation
 * =============================================================================
 * 
 * This file implements the MQSensor class for reading MQ-series gas sensors.
 * 
 * MQ Sensor Working Principle:
 *   - MQ sensors contain a heating element and a sensing element
 *   - The sensing element's resistance (Rs) changes based on gas concentration
 *   - We measure Rs using a voltage divider with a known load resistor (RL)
 *   - The ratio Rs/Ro (where Ro is resistance in clean air) determines PPM
 * 
 * Circuit:
 *   VCC (5V) ─── Rs (sensor) ─┬─ RL (load resistor) ─── GND
 *                             │
 *                         Arduino Analog Pin (measures voltage here)
 * 
 * PPM Calculation:
 *   - Based on datasheet curves: log10(Rs/Ro) = m * log10(PPM) + b
 *   - Rearranged: PPM = 10^((log10(Rs/Ro) - b) / m)
 * 
 * =============================================================================
 */

#include "MQSensor.hpp"
#include <math.h>

// =============================================================================
// CONSTRUCTOR
// =============================================================================

/**
 * Create an MQ sensor instance.
 * 
 * @param pin      Arduino analog pin connected to sensor output
 * @param rl_ohms  Load resistor value in ohms (typically 10k-25k)
 * @param ro_kohm  Known Ro value in kilo-ohms (-1 if needs calibration)
 */
MQSensor::MQSensor(uint8_t pin, float rl_ohms, float ro_kohm)
: _pin(pin), _rl_ohms(rl_ohms), _ro_kohm(ro_kohm) {}

// =============================================================================
// INITIALIZATION
// =============================================================================

/**
 * Initialize the sensor pin as input.
 * Call this in setup() before reading any values.
 */
void MQSensor::begin() {
    pinMode(_pin, INPUT);
}

// =============================================================================
// RAW READING FUNCTIONS
// =============================================================================

/**
 * Read a single raw ADC value from the sensor.
 * 
 * @return ADC value (0-1023 for 10-bit Arduino ADC)
 */
int MQSensor::readRaw() const {
    return analogRead(_pin);
}

/**
 * Read multiple samples and return the average.
 * Averaging reduces noise and provides more stable readings.
 * 
 * @param samples   Number of samples to average (default: 8)
 * @param delay_ms  Delay between samples in milliseconds (default: 5)
 * @return          Averaged ADC value
 */
int MQSensor::readAvg(int samples, unsigned long delay_ms) const {
    long sum = 0;
    for (int i = 0; i < samples; i++) {
        sum += analogRead(_pin);
        delay(delay_ms);
    }
    return sum / samples;
}

// =============================================================================
// VOLTAGE & RESISTANCE CALCULATIONS
// =============================================================================

/**
 * Convert ADC value to voltage.
 * 
 * @param adc  Raw ADC reading (0-1023)
 * @return     Voltage in volts (0.0 - 5.0)
 */
float MQSensor::adcToVoltage(int adc) const {
    return (adc * ADC_REF) / ADC_MAX;
}

/**
 * Compute sensor resistance (Rs) from ADC reading.
 * 
 * Using voltage divider formula:
 *   Vout = VCC * RL / (Rs + RL)
 *   Solving for Rs: Rs = RL * (VCC - Vout) / Vout
 *   In ADC terms: Rs = RL * (ADC_MAX - adc) / adc
 * 
 * @param adc  Raw or averaged ADC reading
 * @return     Sensor resistance in kilo-ohms (kΩ)
 *             Returns 999.9 if sensor is disconnected (adc=0)
 *             Returns 0.01 if sensor is saturated (adc=max)
 */
float MQSensor::computeRsKohm(int adc) const {
    // Safety checks for edge cases
    if (adc <= 0) return 999.9f;      // Open circuit / disconnected sensor
    if (adc >= ADC_MAX) return 0.01f; // Saturated / very high gas concentration
    
    // Calculate Rs using voltage divider formula
    // Result in kΩ (divide by 1000 to convert from Ω)
    return ((ADC_MAX - adc) / (float)adc) * (_rl_ohms / 1000.0f);
}

/**
 * Measure average Rs in clean air for calibration.
 * Should be called after sensor warmup in fresh outdoor air.
 * 
 * @param samples          Number of samples to average (default: 50)
 * @param delayMsBetween   Delay between samples (default: 50ms)
 * @return                 Average Rs in kilo-ohms
 */
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

// =============================================================================
// RATIO CALCULATION
// =============================================================================

/**
 * Calculate Rs/Ro ratio.
 * This ratio is the key value used in PPM calculations.
 * 
 * @param rs_kohm  Current sensor resistance in kilo-ohms
 * @return         Rs/Ro ratio, or -1.0 if Ro is not calibrated
 */
float MQSensor::rsOverRo(float rs_kohm) const {
    // Check if calibration was done
    if (_ro_kohm <= 0 || rs_kohm <= 0) return -1.0f;
    return rs_kohm / _ro_kohm;
}

// =============================================================================
// CALIBRATION
// =============================================================================

/**
 * Calibrate the sensor using known clean air Rs/Ro ratio.
 * Each MQ sensor has a specific clean air ratio from its datasheet:
 *   - MQ-4:   ~4.4 (Methane)
 *   - MQ-136: ~3.6 (H2S)
 *   - MQ-8:   ~70  (Hydrogen)
 *   - MQ-135: ~3.6 (Air Quality)
 * 
 * @param cleanAirRsRoRatio  Datasheet clean air Rs/Ro value
 * @param samples            Number of samples for averaging (default: 100)
 * @param delayMs            Delay between samples (default: 10ms)
 */
void MQSensor::calibrateFromCleanAirRatio(
    float cleanAirRsRoRatio,
    int samples,
    unsigned long delayMs
) {
    // Measure current Rs in clean air
    float rs = measureRsKohmInCleanAir(samples, delayMs);
    
    // Calculate Ro: if Rs/Ro = ratio, then Ro = Rs / ratio
    if (rs > 0 && cleanAirRsRoRatio > 0) {
        _ro_kohm = rs / cleanAirRsRoRatio;
    }
}

// =============================================================================
// PPM CALCULATION
// =============================================================================

/**
 * Calculate gas concentration in PPM from Rs/Ro ratio.
 * Uses the log-log relationship from datasheet curves.
 * 
 * Formula: log10(Rs/Ro) = m * log10(PPM) + b
 * Solved:  PPM = 10^((log10(Rs/Ro) - b) / m)
 * 
 * @param rs_ro  Rs/Ro ratio
 * @param m      Slope from datasheet curve (negative for most sensors)
 * @param b      Y-intercept from datasheet curve
 * @return       Gas concentration in PPM
 */
float MQSensor::calculatePPM(float rs_ro, float m, float b) const {
    // Validate inputs
    if (rs_ro <= 0 || m == 0) return 0.0f;
    
    // Apply log-log formula
    float log_ppm = (log10(rs_ro) - b) / m;
    return pow(10.0f, log_ppm);
}

/**
 * Read sensor and return PPM value.
 * Combines reading, Rs calculation, and PPM conversion.
 * 
 * @param m  Slope from datasheet curve
 * @param b  Y-intercept from datasheet curve
 * @return   Gas concentration in PPM, or -1 if sensor error
 */
float MQSensor::readPPM(float m, float b) {
    // Check if sensor is calibrated
    if (_ro_kohm <= 0) return 0.0f;

    // Read averaged ADC value
    int avgAdc = readAvg();
    
    // Detect unpowered or disconnected sensor
    // Normal readings are typically 100-900 range
    // Floating/disconnected inputs read very low (<10) or very high (>1000)
    if (avgAdc < 10 || avgAdc > 1000) {
        return -1.0f;  // Indicate sensor error
    }
    
    // Calculate resistance and ratio
    float rs = computeRsKohm(avgAdc);
    float ratio = rsOverRo(rs);

    // Convert to PPM
    return calculatePPM(ratio, m, b);
}

/**
 * Read sensor with exponential moving average (EMA) smoothing.
 * Provides more stable readings by filtering out noise and spikes.
 * 
 * EMA Formula: new_avg = alpha * current + (1 - alpha) * old_avg
 * Lower alpha = more smoothing (slower response)
 * Higher alpha = less smoothing (faster response)
 * 
 * @param m  Slope from datasheet curve
 * @param b  Y-intercept from datasheet curve
 * @return   Smoothed gas concentration in PPM
 */
float MQSensor::readPPMSmoothed(float m, float b) {
    float rawPPM = readPPM(m, b);
    
    // If sensor error, return last good value
    if (rawPPM < 0) {
        return _ema_ppm > 0 ? _ema_ppm : 0.0f;
    }
    
    // Initialize EMA on first valid reading
    if (_ema_ppm < 0) {
        _ema_ppm = rawPPM;
    } else {
        // SPIKE REJECTION: Ignore readings that are much higher than average
        // This prevents brief noise from corrupting the average
        if (rawPPM > _ema_ppm * SPIKE_THRESHOLD && _ema_ppm > 0.1f) {
            // Spike detected - keep current average, skip update
            return _ema_ppm;
        }
        
        // Apply EMA filter
        // EMA_ALPHA is defined in header (default: 0.1 = heavy smoothing)
        _ema_ppm = EMA_ALPHA * rawPPM + (1.0f - EMA_ALPHA) * _ema_ppm;
    }
    
    return _ema_ppm;
}
