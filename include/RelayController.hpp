/**
 * =============================================================================
 * RELAY CONTROLLER - 8-Channel Relay Module
 * =============================================================================
 * 
 * Controls an 8-channel relay module for the Outer Science Box.
 * Used for switching 12V devices like pumps, valves, motors, etc.
 * 
 * Hardware:
 *   - 6-Channel Relay Module (active LOW logic)
 *   - Arduino Mega 2560
 *   - 12V power supply for relay loads
 * 
 * Wiring Diagram:
 *   ┌─────────────────────────────────────────────────────────────────────┐
 *   │  RELAY MODULE                                                       │
 *   │  ┌─────┬─────┬─────┬─────┬─────┬─────┬─────┬─────┐                 │
 *   │  │ IN1 │ IN2 │ IN3 │ IN4 │ IN5 │ IN6 │ IN7 │ IN8 │                 │
 *   │  └──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┴──┬──┘                 │
 *   │     │     │     │     │     │     │     │     │                     │
 *   │     │ Arduino Digital Pins              │     │                     │
 *   │     │  46   44   45   43   41   39   37   35  │                     │
 *   │                                                                     │
 *   │  VCC ─────> Arduino 5V                                              │
 *   │  GND ─────> Arduino GND                                             │
 *   └─────────────────────────────────────────────────────────────────────┘
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
// Pins: IN1=46, IN2=44, IN3=45, IN4=43, IN5=41, IN6=39, IN7=37, IN8=35

#define RELAY_1_PIN  46   // Relay 1 (IN1): Pin 46
#define RELAY_2_PIN  44   // Relay 2 (IN2): Pin 44
#define RELAY_3_PIN  45   // Relay 3 (IN3): Pin 45
#define RELAY_4_PIN  43   // Relay 4 (IN4): Pin 43
#define RELAY_5_PIN  41   // Relay 5 (IN5): Pin 41
#define RELAY_6_PIN  39   // Relay 6 (IN6): Pin 39
#define RELAY_7_PIN  37   // Relay 7 (IN7): Pin 37
#define RELAY_8_PIN  35   // Relay 8 (IN8): Pin 35

#define NUM_RELAYS 8      // Total number of relays

// =============================================================================
// MOTOR RELAY ASSIGNMENTS
// =============================================================================
// Platform 1 - Main arm (Motor 1)
#define PLATFORM1_UP_RELAY    1   // Relay 1: Platform 1 moves UP
#define PLATFORM1_DOWN_RELAY  2   // Relay 2: Platform 1 moves DOWN

// Platform 2 - Drill platform (Motor 2)
#define PLATFORM2_UP_RELAY    4   // Relay 4: Platform 2 moves UP
#define PLATFORM2_DOWN_RELAY  3   // Relay 3: Platform 2 moves DOWN

// Drill (Motor 3)
#define DRILL_RELAY           5   // Relay 5: Drill ON/OFF
#define LIFT_RELAY            6   // Relay 6: Lift mechanism

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
        RELAY_1_PIN,   // relayPins[0] = Pin 46 (IN1)
        RELAY_2_PIN,   // relayPins[1] = Pin 44 (IN2)
        RELAY_3_PIN,   // relayPins[2] = Pin 45 (IN3)
        RELAY_4_PIN,   // relayPins[3] = Pin 43 (IN4)
        RELAY_5_PIN,   // relayPins[4] = Pin 41 (IN5)
        RELAY_6_PIN,   // relayPins[5] = Pin 39 (IN6)
        RELAY_7_PIN,   // relayPins[6] = Pin 37 (IN7)
        RELAY_8_PIN    // relayPins[7] = Pin 35 (IN8)
    };
    
    // Track current state of each relay (true = ON, false = OFF)
    bool relayStates[NUM_RELAYS] = {false, false, false, false, false, false, false, false};

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
        Serial.println(F("RelayController: All 8 relays initialized (OFF)"));
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

    // =========================================================================
    // PLATFORM & DRILL MOTOR CONTROL
    // =========================================================================

    /**
     * Move Platform 1 (main arm) UP for specified duration.
     * @param durationMs How long to move (milliseconds)
     */
    void platform1Up(unsigned long durationMs) {
        Serial.println(F("Platform 1: Moving UP..."));
        pulseOn(PLATFORM1_UP_RELAY, durationMs);
    }

    /**
     * Move Platform 1 (main arm) DOWN for specified duration.
     * @param durationMs How long to move (milliseconds)
     */
    void platform1Down(unsigned long durationMs) {
        Serial.println(F("Platform 1: Moving DOWN..."));
        pulseOn(PLATFORM1_DOWN_RELAY, durationMs);
    }

    /**
     * Move Platform 2 (drill platform) UP for specified duration.
     * @param durationMs How long to move (milliseconds)
     */
    void platform2Up(unsigned long durationMs) {
        Serial.println(F("Platform 2: Moving UP..."));
        pulseOn(PLATFORM2_UP_RELAY, durationMs);
    }

    /**
     * Move Platform 2 (drill platform) DOWN for specified duration.
     * @param durationMs How long to move (milliseconds)
     */
    void platform2Down(unsigned long durationMs) {
        Serial.println(F("Platform 2: Moving DOWN..."));
        pulseOn(PLATFORM2_DOWN_RELAY, durationMs);
    }

    /**
     * Turn the drill ON.
     */
    void drillOn() {
        Serial.println(F("Drill: ON"));
        turnOn(DRILL_RELAY);
    }

    /**
     * Turn the drill OFF.
     */
    void drillOff() {
        Serial.println(F("Drill: OFF"));
        turnOff(DRILL_RELAY);
    }

    /**
     * Activate lift mechanism.
     * @param durationMs How long to activate (milliseconds)
     */
    void liftActivate(unsigned long durationMs) {
        Serial.println(F("Lift mechanism: Activating..."));
        pulseOn(LIFT_RELAY, durationMs);
    }

    // =========================================================================
    // SOIL COLLECTION SEQUENCE
    // =========================================================================

    /**
     * Execute the complete soil collection sequence.
     * 
     * Sequence:
     *   1. Platform 1 DOWN  → Lower main arm
     *   2. Drill ON         → Start drill motor
     *   3. Platform 2 DOWN  → Push drill into soil
     *   4. Wait             → Allow drilling
     *   5. Platform 2 UP    → Retract with soil sample
     *   6. Drill OFF        → Stop drill
     *   7. Platform 1 UP    → Raise main arm
     * 
     * @param p1DownTime   Time for Platform 1 to lower (ms), default 3000
     * @param p1UpTime     Time for Platform 1 to raise (ms), default 3000
     * @param p2DownTime   Time for Platform 2 to lower (ms), default 3000
     * @param p2UpTime     Time for Platform 2 to raise (ms), default 3000
     * @param drillTime    Time to drill in soil (ms), default 2000
     */
    void soilCollectionSequence(
        unsigned long p1DownTime = 3000,
        unsigned long p1UpTime   = 3000,
        unsigned long p2DownTime = 3000,
        unsigned long p2UpTime   = 3000,
        unsigned long drillTime  = 2000
    ) {
        Serial.println(F(""));
        Serial.println(F("==========================================="));
        Serial.println(F("  SOIL COLLECTION SEQUENCE - STARTING"));
        Serial.println(F("==========================================="));
        
        // Step 1: Lower Platform 1 (main arm)
        Serial.println(F("[Step 1/7] Lowering Platform 1..."));
        platform1Down(p1DownTime);
        delay(500);  // Brief pause
        
        // Step 2: Start the drill
        Serial.println(F("[Step 2/7] Starting drill..."));
        drillOn();
        delay(500);  // Let drill spin up
        
        // Step 3: Lower Platform 2 (push drill into soil)
        Serial.println(F("[Step 3/7] Lowering Platform 2 into soil..."));
        platform2Down(p2DownTime);
        
        // Step 4: Wait for drilling
        Serial.println(F("[Step 4/7] Drilling..."));
        delay(drillTime);
        
        // Step 5: Raise Platform 2 (retract with soil)
        Serial.println(F("[Step 5/7] Retracting Platform 2 with soil..."));
        platform2Up(p2UpTime);
        
        // Step 6: Stop drill
        Serial.println(F("[Step 6/7] Stopping drill..."));
        drillOff();
        delay(500);  // Brief pause
        
        // Step 7: Raise Platform 1 (bring sample up)
        Serial.println(F("[Step 7/7] Raising Platform 1..."));
        platform1Up(p1UpTime);
        
        Serial.println(F("==========================================="));
        Serial.println(F("  SOIL COLLECTION COMPLETE!"));
        Serial.println(F("==========================================="));
        Serial.println(F(""));
    }
};

#endif // RELAY_CONTROLLER_HPP

