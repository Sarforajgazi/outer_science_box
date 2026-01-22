/**
 * RelayController.hpp
 * 
 * Controls 6-channel relay module for outer science box
 * Relays are active LOW (common for most relay modules)
 * 
 * Wiring:
 *   - VCC -> 5V (Arduino)
 *   - GND -> GND (Arduino)
 *   - IN1-IN6 -> Digital pins (defined below)
 *   - COM -> 12V power supply positive
 *   - NO -> Device positive terminal (Normally Open)
 *   - NC -> Not used (Normally Closed)
 * 
 * Each device's negative terminal connects to 12V supply ground
 */

#ifndef RELAY_CONTROLLER_HPP
#define RELAY_CONTROLLER_HPP

#include <Arduino.h>

// ========== RELAY PIN DEFINITIONS ==========
// Pins configured for your wiring
#define RELAY_1_PIN  35   // Relay 1 control pin
#define RELAY_2_PIN  37   // Relay 2 control pin
#define RELAY_3_PIN  39   // Relay 3 control pin
#define RELAY_4_PIN  41   // Relay 4 control pin
#define RELAY_5_PIN  43   // Relay 5 control pin
#define RELAY_6_PIN  45   // Relay 6 control pin

// Number of relays
#define NUM_RELAYS 6

// Relay logic (most modules are active LOW)
#define RELAY_ON   LOW
#define RELAY_OFF  HIGH

class RelayController {
private:
    uint8_t relayPins[NUM_RELAYS] = {
        RELAY_1_PIN,
        RELAY_2_PIN,
        RELAY_3_PIN,
        RELAY_4_PIN,
        RELAY_5_PIN,
        RELAY_6_PIN
    };
    
    bool relayStates[NUM_RELAYS] = {false};  // Track current states

public:
    /**
     * Initialize all relay pins as OUTPUT and turn them OFF
     */
    void begin() {
        for (uint8_t i = 0; i < NUM_RELAYS; i++) {
            pinMode(relayPins[i], OUTPUT);
            digitalWrite(relayPins[i], RELAY_OFF);  // Start with all relays OFF
            relayStates[i] = false;
        }
        Serial.println(F("RelayController: All 6 relays initialized (OFF)"));
    }

    /**
     * Turn ON a specific relay (1-6)
     * @param relayNum Relay number (1-6)
     */
    void turnOn(uint8_t relayNum) {
        if (relayNum < 1 || relayNum > NUM_RELAYS) {
            Serial.print(F("Invalid relay number: "));
            Serial.println(relayNum);
            return;
        }
        uint8_t idx = relayNum - 1;
        digitalWrite(relayPins[idx], RELAY_ON);
        relayStates[idx] = true;
        Serial.print(F("Relay "));
        Serial.print(relayNum);
        Serial.println(F(" ON"));
    }

    /**
     * Turn OFF a specific relay (1-6)
     * @param relayNum Relay number (1-6)
     */
    void turnOff(uint8_t relayNum) {
        if (relayNum < 1 || relayNum > NUM_RELAYS) {
            Serial.print(F("Invalid relay number: "));
            Serial.println(relayNum);
            return;
        }
        uint8_t idx = relayNum - 1;
        digitalWrite(relayPins[idx], RELAY_OFF);
        relayStates[idx] = false;
        Serial.print(F("Relay "));
        Serial.print(relayNum);
        Serial.println(F(" OFF"));
    }

    /**
     * Toggle a specific relay
     * @param relayNum Relay number (1-6)
     */
    void toggle(uint8_t relayNum) {
        if (relayNum < 1 || relayNum > NUM_RELAYS) return;
        uint8_t idx = relayNum - 1;
        if (relayStates[idx]) {
            turnOff(relayNum);
        } else {
            turnOn(relayNum);
        }
    }

    /**
     * Turn ON a relay for a specified duration (blocking)
     * @param relayNum Relay number (1-6)
     * @param durationMs Duration in milliseconds
     */
    void pulseOn(uint8_t relayNum, unsigned long durationMs) {
        turnOn(relayNum);
        delay(durationMs);
        turnOff(relayNum);
    }

    /**
     * Turn ALL relays ON
     */
    void allOn() {
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            turnOn(i);
        }
    }

    /**
     * Turn ALL relays OFF
     */
    void allOff() {
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            turnOff(i);
        }
    }

    /**
     * Get current state of a relay
     * @param relayNum Relay number (1-6)
     * @return true if ON, false if OFF
     */
    bool getState(uint8_t relayNum) {
        if (relayNum < 1 || relayNum > NUM_RELAYS) return false;
        return relayStates[relayNum - 1];
    }

    /**
     * Print status of all relays
     */
    void printStatus() {
        Serial.println(F("--- Relay Status ---"));
        for (uint8_t i = 0; i < NUM_RELAYS; i++) {
            Serial.print(F("Relay "));
            Serial.print(i + 1);
            Serial.print(F(": "));
            Serial.println(relayStates[i] ? F("ON") : F("OFF"));
        }
        Serial.println(F("--------------------"));
    }

    /**
     * Activate relays in sequence (useful for dispensing)
     * @param delayBetween Delay between each relay activation (ms)
     * @param onDuration How long each relay stays ON (ms)
     */
    void sequentialActivation(unsigned long delayBetween, unsigned long onDuration) {
        Serial.println(F("Sequential relay activation started..."));
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            pulseOn(i, onDuration);
            if (i < NUM_RELAYS) {
                delay(delayBetween);
            }
        }
        Serial.println(F("Sequential activation complete."));
    }
};

#endif // RELAY_CONTROLLER_HPP
