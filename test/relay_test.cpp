/**
 * =============================================================================
 * RELAY TEST FILE
 * =============================================================================
 * 
 * Interactive test program for the 6-channel relay module.
 * Use this to verify relay wiring and operation before integration.
 * 
 * How to Use:
 *   1. Upload this file to Arduino Mega
 *   2. Open Serial Monitor at 9600 baud
 *   3. Send commands to control relays
 * 
 * Commands:
 *   ┌─────────┬────────────────────────────────────────────────┐
 *   │ Command │ Action                                         │
 *   ├─────────┼────────────────────────────────────────────────┤
 *   │ 1-6     │ Toggle that specific relay                     │
 *   │ a       │ Turn ALL relays ON                             │
 *   │ o       │ Turn ALL relays OFF                            │
 *   │ s       │ Sequential test (each relay ON for 1 second)   │
 *   │ t       │ Print current status of all relays             │
 *   └─────────┴────────────────────────────────────────────────┘
 * 
 * Hardware:
 *   - Relay 1: Pin 35
 *   - Relay 2: Pin 37
 *   - Relay 3: Pin 39
 *   - Relay 4: Pin 41
 *   - Relay 5: Pin 43
 *   - Relay 6: Pin 45
 * 
 * Safety:
 *   - Relay loads should be disconnected during initial testing
 *   - Listen for relay click sounds to verify operation
 *   - Watch for relay LEDs (if present on module)
 * 
 * =============================================================================
 */

#include <Arduino.h>
#include "RelayController.hpp"

// Create relay controller instance
RelayController relays;

/**
 * Setup - Runs once on power-up
 */
void setup() {
    // Initialize serial communication
    Serial.begin(9600);
    
    // Wait for serial port to connect (max 2 seconds)
    while (!Serial && millis() < 2000) {
        delay(10);
    }

    // Print welcome banner
    Serial.println(F("================================"));
    Serial.println(F("   6-CHANNEL RELAY TEST"));
    Serial.println(F("   Team Obseract Rover"));
    Serial.println(F("================================"));
    Serial.println(F("Pins: 35, 37, 39, 41, 43, 45"));
    Serial.println(F(""));

    // Initialize all relay pins (starts with all OFF)
    relays.begin();

    // Print command help
    Serial.println(F(""));
    Serial.println(F("Commands:"));
    Serial.println(F("  1-6  -> Toggle relay 1-6"));
    Serial.println(F("  a    -> All ON"));
    Serial.println(F("  o    -> All OFF"));
    Serial.println(F("  s    -> Sequential test"));
    Serial.println(F("  t    -> Print status"));
    Serial.println(F("================================"));
}

/**
 * Main Loop - Runs repeatedly
 * 
 * Checks for serial input and executes commands.
 */
void loop() {
    // Check if there's data available from Serial Monitor
    if (Serial.available()) {
        // Read the command character
        char cmd = Serial.read();
        
        // Clear any extra characters (like newline '\n' or carriage return '\r')
        // This prevents accidental triggering on the next loop
        while (Serial.available()) {
            Serial.read();
        }

        // Process the command
        switch (cmd) {
            // =========================================================
            // COMMANDS 1-6: Toggle individual relays
            // =========================================================
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
                // Convert character to number: '1' -> 1, '2' -> 2, etc.
                relays.toggle(cmd - '0');
                break;
            
            // =========================================================
            // COMMAND 'a': Turn ALL relays ON
            // =========================================================
            case 'a':
            case 'A':
                Serial.println(F("\n>> ALL RELAYS ON"));
                relays.allOn();
                break;
            
            // =========================================================
            // COMMAND 'o': Turn ALL relays OFF
            // =========================================================
            case 'o':
            case 'O':
                Serial.println(F("\n>> ALL RELAYS OFF"));
                relays.allOff();
                break;
            
            // =========================================================
            // COMMAND 's': Sequential test
            // Each relay turns ON for 1 second (1000ms)
            // 200ms delay between relays
            // Total time: ~7.2 seconds
            // =========================================================
            case 's':
            case 'S':
                Serial.println(F("\n>> SEQUENTIAL TEST"));
                Serial.println(F("Each relay ON for 1 second..."));
                
                // Loop through relays 1-6
                for (int i = 1; i <= 6; i++) {
                    relays.pulseOn(i, 1000);  // ON for 1000ms (1 second)
                    delay(200);               // 200ms gap between relays
                }
                
                Serial.println(F("Test complete!"));
                break;
            
            // =========================================================
            // COMMAND 't': Print status
            // =========================================================
            case 't':
            case 'T':
                relays.printStatus();
                break;
            
            // =========================================================
            // UNKNOWN COMMAND: Ignore
            // =========================================================
            default:
                // Don't print anything for unknown commands
                // (avoids spamming on newline characters)
                break;
        }
    }
}
