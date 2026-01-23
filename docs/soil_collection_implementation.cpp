/**
 * =============================================================================
 * SOIL COLLECTION SEQUENCE - Implementation Reference
 * =============================================================================
 * 
 * This file contains the motor functions and soil collection sequence
 * that can be added to RelayController.hpp when ready.
 * 
 * =============================================================================
 */

// =============================================================================
// ADD TO RelayController.hpp AFTER PIN DEFINITIONS (~line 62)
// =============================================================================

// MOTOR RELAY ASSIGNMENTS
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
// ADD THESE FUNCTIONS INSIDE RelayController CLASS (before the closing brace)
// =============================================================================

// --- PLATFORM 1 CONTROL ---

void platform1Up(unsigned long durationMs) {
    Serial.println(F("Platform 1: Moving UP..."));
    pulseOn(PLATFORM1_UP_RELAY, durationMs);
}

void platform1Down(unsigned long durationMs) {
    Serial.println(F("Platform 1: Moving DOWN..."));
    pulseOn(PLATFORM1_DOWN_RELAY, durationMs);
}

// --- PLATFORM 2 CONTROL ---

void platform2Up(unsigned long durationMs) {
    Serial.println(F("Platform 2: Moving UP..."));
    pulseOn(PLATFORM2_UP_RELAY, durationMs);
}

void platform2Down(unsigned long durationMs) {
    Serial.println(F("Platform 2: Moving DOWN..."));
    pulseOn(PLATFORM2_DOWN_RELAY, durationMs);
}

// --- DRILL CONTROL ---

void drillOn() {
    Serial.println(F("Drill: ON"));
    turnOn(DRILL_RELAY);
}

void drillOff() {
    Serial.println(F("Drill: OFF"));
    turnOff(DRILL_RELAY);
}

void liftActivate(unsigned long durationMs) {
    Serial.println(F("Lift mechanism: Activating..."));
    pulseOn(LIFT_RELAY, durationMs);
}

// --- SOIL COLLECTION SEQUENCE ---

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
 * @param platformMoveTime  Time for platform movements (ms), default 3000
 * @param drillTime         Time to drill in soil (ms), default 2000
 */
void soilCollectionSequence(
    unsigned long platformMoveTime = 3000,
    unsigned long drillTime = 2000
) {
    Serial.println(F(""));
    Serial.println(F("==========================================="));
    Serial.println(F("  SOIL COLLECTION SEQUENCE - STARTING"));
    Serial.println(F("==========================================="));
    
    // Step 1: Lower Platform 1 (main arm)
    Serial.println(F("[Step 1/7] Lowering Platform 1..."));
    platform1Down(platformMoveTime);
    delay(500);
    
    // Step 2: Start the drill
    Serial.println(F("[Step 2/7] Starting drill..."));
    drillOn();
    delay(500);
    
    // Step 3: Lower Platform 2 (push drill into soil)
    Serial.println(F("[Step 3/7] Lowering Platform 2 into soil..."));
    platform2Down(platformMoveTime);
    
    // Step 4: Wait for drilling
    Serial.println(F("[Step 4/7] Drilling..."));
    delay(drillTime);
    
    // Step 5: Raise Platform 2 (retract with soil)
    Serial.println(F("[Step 5/7] Retracting Platform 2 with soil..."));
    platform2Up(platformMoveTime);
    
    // Step 6: Stop drill
    Serial.println(F("[Step 6/7] Stopping drill..."));
    drillOff();
    delay(500);
    
    // Step 7: Raise Platform 1 (bring sample up)
    Serial.println(F("[Step 7/7] Raising Platform 1..."));
    platform1Up(platformMoveTime);
    
    Serial.println(F("==========================================="));
    Serial.println(F("  SOIL COLLECTION COMPLETE!"));
    Serial.println(F("==========================================="));
}


// =============================================================================
// ADD TO relay_test.cpp IN THE switch STATEMENT
// =============================================================================

// Add this case before the "default:" case:

case 'c':
case 'C':
    relays.soilCollectionSequence();
    break;

// Also update the help text in setup() to include:
// Serial.println(F("  c    -> Soil collection sequence"));
