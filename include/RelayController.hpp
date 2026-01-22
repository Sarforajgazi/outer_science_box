/**
 * =============================================================================
 * RELAY CONTROLLER - 6-Channel Relay Module
 * =============================================================================
 * 
 * Controls a 6-channel relay module for the Outer Science Box.
 * Used for switching 12V devices like pumps, valves, motors, etc.
 * 
 * Hardware:
 *   - 6-Channel Relay Module (active LOW logic)
 *   - Arduino Mega 2560
 *   - 12V power supply for relay loads
 * 
 * Wiring Diagram:
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  RELAY MODULE                                               │
 *   │  ┌─────┬─────┬─────┬─────┬─────┬─────┐                     │
 *   │  │ IN1 │ IN2 │ IN3 │ IN4 │ IN5 │ IN6 │                     │
 *   │  └──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┘                     │
 *   │     │     │     │     │     │     │                         │
 *   │     │ Arduino Digital Pins        │                         │
 *   │     │  35   37   39   41   43   45│                         │
 *   │                                                             │
 *   │  VCC ─────> Arduino 5V                                      │
 *   │  GND ─────> Arduino GND                                     │
 *   └─────────────────────────────────────────────────────────────┘
 * 
 *   Each relay has 3 terminals:
 *     COM (Common)  ───> 12V Power Supply (+)
 *     NO  (Normally Open) ───> Device (+) terminal
 *     NC  (Normally Closed) ───> Not used
 *   
 *     Device (-) terminal ───> 12V Power Supply (-)
 * 
 * Logic:
 *   - Most relay modules are ACTIVE LOW
 *   - LOW signal = Relay ON (closed)
 *   - HIGH signal = Relay OFF (open)
 * 
 * =============================================================================
 */

#ifndef RELAY_CONTROLLER_HPP
#define RELAY_CONTROLLER_HPP

#include <Arduino.h>

// =============================================================================
// PIN DEFINITIONS
// =============================================================================
// Pins are on the odd-numbered digital pins (35, 37, 39, 41, 43, 45)
// These are easy to remember and keep wiring organized

#define RELAY_1_PIN  35   // Relay 1: Pin 35
#define RELAY_2_PIN  37   // Relay 2: Pin 37
#define RELAY_3_PIN  39   // Relay 3: Pin 39
#define RELAY_4_PIN  41   // Relay 4: Pin 41
#define RELAY_5_PIN  43   // Relay 5: Pin 43
#define RELAY_6_PIN  45   // Relay 6: Pin 45

#define NUM_RELAYS 6      // Total number of relays

// =============================================================================
// RELAY LOGIC CONSTANTS
// =============================================================================
// Most relay modules use ACTIVE LOW logic:
//   LOW  = Transistor ON = Relay coil energized = Contact closed
//   HIGH = Transistor OFF = Relay coil off = Contact open

#define RELAY_ON   LOW    // Signal to turn relay ON (active LOW)
#define RELAY_OFF  HIGH   // Signal to turn relay OFF

// =============================================================================
// RELAY CONTROLLER CLASS
// =============================================================================

class RelayController {
private:
    // =========================================================================
    // MEMBER VARIABLES
    // =========================================================================
    
    // Array of pin numbers for each relay (0-indexed)
    uint8_t relayPins[NUM_RELAYS] = {
        RELAY_1_PIN,   // relayPins[0] = Pin 35
        RELAY_2_PIN,   // relayPins[1] = Pin 37
        RELAY_3_PIN,   // relayPins[2] = Pin 39
        RELAY_4_PIN,   // relayPins[3] = Pin 41
        RELAY_5_PIN,   // relayPins[4] = Pin 43
        RELAY_6_PIN    // relayPins[5] = Pin 45
    };
    
    // Track current state of each relay (true = ON, false = OFF)
    bool relayStates[NUM_RELAYS] = {false, false, false, false, false, false};

public:
    // =========================================================================
    // INITIALIZATION
    // =========================================================================
    
    /**
     * Initialize all relay pins as OUTPUT and turn them OFF.
     * 
     * IMPORTANT: Call this in setup() before using any relay functions.
     *            Relays start in OFF state to prevent accidental activation.
     */
    void begin() {
        for (uint8_t i = 0; i < NUM_RELAYS; i++) {
            pinMode(relayPins[i], OUTPUT);
            digitalWrite(relayPins[i], RELAY_OFF);  // Start with all relays OFF
            relayStates[i] = false;
        }
        Serial.println(F("RelayController: All 6 relays initialized (OFF)"));
    }

    // =========================================================================
    // INDIVIDUAL RELAY CONTROL
    // =========================================================================

    /**
     * Turn ON a specific relay.
     * 
     * @param relayNum Relay number (1-6, NOT 0-indexed!)
     *                 Invalid numbers are rejected with error message.
     */
    void turnOn(uint8_t relayNum) {
        // Validate relay number (1-6)
        if (relayNum < 1 || relayNum > NUM_RELAYS) {
            Serial.print(F("Invalid relay number: "));
            Serial.println(relayNum);
            return;
        }
        
        // Convert to 0-indexed array position
        uint8_t idx = relayNum - 1;
        
        // Activate relay (LOW = ON for active-low modules)
        digitalWrite(relayPins[idx], RELAY_ON);
        relayStates[idx] = true;
        
        // Log action
        Serial.print(F("Relay "));
        Serial.print(relayNum);
        Serial.println(F(" ON"));
    }

    /**
     * Turn OFF a specific relay.
     * 
     * @param relayNum Relay number (1-6, NOT 0-indexed!)
     *                 Invalid numbers are rejected with error message.
     */
    void turnOff(uint8_t relayNum) {
        // Validate relay number (1-6)
        if (relayNum < 1 || relayNum > NUM_RELAYS) {
            Serial.print(F("Invalid relay number: "));
            Serial.println(relayNum);
            return;
        }
        
        // Convert to 0-indexed array position
        uint8_t idx = relayNum - 1;
        
        // Deactivate relay (HIGH = OFF for active-low modules)
        digitalWrite(relayPins[idx], RELAY_OFF);
        relayStates[idx] = false;
        
        // Log action
        Serial.print(F("Relay "));
        Serial.print(relayNum);
        Serial.println(F(" OFF"));
    }

    /**
     * Toggle a relay (ON→OFF or OFF→ON).
     * 
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
     * Turn ON a relay for a specified duration, then turn OFF.
     * 
     * WARNING: This function is BLOCKING - Arduino cannot do anything else
     *          while the relay is pulsing. For non-blocking, use millis().
     * 
     * @param relayNum   Relay number (1-6)
     * @param durationMs Duration in milliseconds (1000 = 1 second)
     */
    void pulseOn(uint8_t relayNum, unsigned long durationMs) {
        turnOn(relayNum);        // Turn ON
        delay(durationMs);       // Wait for specified duration
        turnOff(relayNum);       // Turn OFF
    }

    // =========================================================================
    // BULK RELAY CONTROL
    // =========================================================================

    /**
     * Turn ALL 6 relays ON.
     * Useful for system-wide activation or testing.
     */
    void allOn() {
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            turnOn(i);
        }
    }

    /**
     * Turn ALL 6 relays OFF.
     * Useful for emergency stop or reset.
     */
    void allOff() {
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            turnOff(i);
        }
    }

    // =========================================================================
    // STATUS FUNCTIONS
    // =========================================================================

    /**
     * Get current state of a relay.
     * 
     * @param relayNum Relay number (1-6)
     * @return true if relay is ON, false if OFF or invalid number
     */
    bool getState(uint8_t relayNum) {
        if (relayNum < 1 || relayNum > NUM_RELAYS) return false;
        return relayStates[relayNum - 1];
    }

    /**
     * Print status of all relays to Serial.
     * Useful for debugging and monitoring.
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

    // =========================================================================
    // SEQUENCE FUNCTIONS
    // =========================================================================

    /**
     * Activate relays in sequence (1 → 2 → 3 → 4 → 5 → 6).
     * 
     * Useful for:
     *   - Sequential dispensing (one chemical at a time)
     *   - Testing all relays
     *   - Visual demonstrations
     * 
     * @param delayBetween Delay between each relay activation (ms)
     * @param onDuration   How long each relay stays ON (ms)
     * 
     * Total time = NUM_RELAYS × onDuration + (NUM_RELAYS-1) × delayBetween
     * Example: 6 × 1000 + 5 × 200 = 7000ms = 7 seconds
     */
    void sequentialActivation(unsigned long delayBetween, unsigned long onDuration) {
        Serial.println(F("Sequential relay activation started..."));
        
        for (uint8_t i = 1; i <= NUM_RELAYS; i++) {
            pulseOn(i, onDuration);  // Turn ON for specified duration
            
            // Add delay between relays (except after the last one)
            if (i < NUM_RELAYS) {
                delay(delayBetween);
            }
        }
        
        Serial.println(F("Sequential activation complete."));
    }
};

#endif // RELAY_CONTROLLER_HPP
