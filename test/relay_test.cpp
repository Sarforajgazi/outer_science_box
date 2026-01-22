/**
 * RELAY TEST FILE
 * 
 * Upload this to test your 6-channel relay module.
 * Open Serial Monitor at 9600 baud and use these commands:
 * 
 * Commands:
 *   1-6     -> Toggle that relay
 *   a       -> Turn ALL ON
 *   o       -> Turn ALL OFF
 *   s       -> Sequential test (each relay 1 second)
 *   t       -> Print status
 */

#include <Arduino.h>
#include "RelayController.hpp"

RelayController relays;

void setup() {
    Serial.begin(9600);
    while (!Serial && millis() < 2000) {
        delay(10);
    }

    Serial.println(F("================================"));
    Serial.println(F("   6-CHANNEL RELAY TEST"));
    Serial.println(F("================================"));
    Serial.println(F("Pins: 35, 37, 39, 41, 43, 45"));
    Serial.println(F(""));

    relays.begin();

    Serial.println(F(""));
    Serial.println(F("Commands:"));
    Serial.println(F("  1-6  -> Toggle relay 1-6"));
    Serial.println(F("  a    -> All ON"));
    Serial.println(F("  o    -> All OFF"));
    Serial.println(F("  s    -> Sequential test"));
    Serial.println(F("  t    -> Print status"));
    Serial.println(F("================================"));
}

void loop() {
    if (Serial.available()) {
        char cmd = Serial.read();
        
        // Clear any extra characters (like newline)
        while (Serial.available()) {
            Serial.read();
        }

        switch (cmd) {
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
                relays.toggle(cmd - '0');
                break;
            
            case 'a':
            case 'A':
                Serial.println(F("\n>> ALL RELAYS ON"));
                relays.allOn();
                break;
            
            case 'o':
            case 'O':
                Serial.println(F("\n>> ALL RELAYS OFF"));
                relays.allOff();
                break;
            
            case 's':
            case 'S':
                Serial.println(F("\n>> SEQUENTIAL TEST"));
                Serial.println(F("Each relay ON for 1 second..."));
                for (int i = 1; i <= 6; i++) {
                    relays.pulseOn(i, 1000);
                    delay(200);
                }
                Serial.println(F("Test complete!"));
                break;
            
            case 't':
            case 'T':
                relays.printStatus();
                break;
            
            default:
                break;
        }
    }
}
