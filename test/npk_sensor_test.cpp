/**
 * =============================================================================
 * NPK 7-in-1 SENSOR TEST
 * =============================================================================
 * 
 * Test program for the NPK 7-in-1 soil sensor via RS485 Modbus RTU.
 * 
 * Hardware Setup (Arduino Mega):
 *   - RS485 Module DI  -> TX1 (Pin 18)
 *   - RS485 Module RO  -> RX1 (Pin 19)
 *   - RS485 Module DE  -> Pin 2
 *   - RS485 Module RE  -> Pin 2 (connect together with DE)
 *   - RS485 Module VCC -> 5V
 *   - RS485 Module GND -> GND
 *   - NPK Sensor VCC   -> 12V or 24V DC (check your sensor)
 *   - NPK Sensor GND   -> GND (common ground with Arduino)
 *   - NPK Sensor A+    -> RS485 Module A+
 *   - NPK Sensor B-    -> RS485 Module B-
 * 
 * Upload: pio run -e npk_test -t upload
 * Monitor: pio device monitor -b 9600
 * 
 * Author: Team Obseract
 * =============================================================================
 */

#include <Arduino.h>
#include "NPKSensor.hpp"

// =============================================================================
// CONFIGURATION - Adjust these settings for your setup
// =============================================================================

// RS485 Control Pin (DE and RE tied together)
#define RS485_DE_PIN        2

// Serial port for RS485 communication
// Serial1: TX1=18, RX1=19
// Serial2: TX2=16, RX2=17
// Serial3: TX3=14, RX3=15
#define NPK_SERIAL          Serial1

// NPK Sensor Settings
#define NPK_SLAVE_ADDRESS   0x01    // Default address (try 0x01, 0x02, or 0xFF)
#define NPK_BAUD_RATE       4800    // Common: 4800, 9600, 19200

// Reading interval in milliseconds
#define READ_INTERVAL       2000

// =============================================================================
// GLOBALS
// =============================================================================

NPKSensor npkSensor(&NPK_SERIAL, RS485_DE_PIN, NPK_SLAVE_ADDRESS, NPK_BAUD_RATE);
unsigned long lastReadTime = 0;
bool csvMode = false;

// =============================================================================
// SETUP
// =============================================================================

void setup() {
    // Initialize debug serial
    Serial.begin(9600);
    while (!Serial && millis() < 3000);  // Wait for Serial (timeout 3 sec)
    
    Serial.println();
    Serial.println("==============================================");
    Serial.println("   NPK 7-in-1 Sensor Test (RS485 Modbus)");
    Serial.println("==============================================");
    Serial.println();
    Serial.println("Hardware Configuration:");
    Serial.print("  - RS485 DE/RE Pin: "); Serial.println(RS485_DE_PIN);
    Serial.print("  - Serial Port:     Serial1 (TX1=18, RX1=19)");
    Serial.println();
    Serial.print("  - Baud Rate:       "); Serial.println(NPK_BAUD_RATE);
    Serial.print("  - Slave Address:   0x"); Serial.println(NPK_SLAVE_ADDRESS, HEX);
    Serial.println();
    
    // Initialize NPK sensor
    npkSensor.begin();
    
    Serial.println("Commands:");
    Serial.println("  'r' - Read sensor once");
    Serial.println("  'c' - Toggle CSV mode (continuous output)");
    Serial.println("  'h' - Print CSV header");
    Serial.println("  '1' - Set slave address to 0x01");
    Serial.println("  '2' - Set slave address to 0x02");
    Serial.println();
    Serial.println("Starting automatic readings every 2 seconds...");
    Serial.println();
    
    // Print CSV header
    npkSensor.printCSVHeader();
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
    // Check for serial commands
    if (Serial.available()) {
        char cmd = Serial.read();
        
        switch (cmd) {
            case 'r':
            case 'R': {
                Serial.println("\n[Manual Read]");
                NPKData data = npkSensor.read();
                npkSensor.printData(data);
                break;
            }
            
            case 'c':
            case 'C':
                csvMode = !csvMode;
                Serial.print("CSV Mode: "); 
                Serial.println(csvMode ? "ON (continuous)" : "OFF (formatted)");
                if (csvMode) {
                    npkSensor.printCSVHeader();
                }
                break;
            
            case 'h':
            case 'H':
                npkSensor.printCSVHeader();
                break;
            
            case '1':
                npkSensor.setSlaveAddress(0x01);
                Serial.println("Slave address set to 0x01");
                break;
            
            case '2':
                npkSensor.setSlaveAddress(0x02);
                Serial.println("Slave address set to 0x02");
                break;
        }
    }
    
    // Automatic reading at interval
    if (millis() - lastReadTime >= READ_INTERVAL) {
        lastReadTime = millis();
        
        NPKData data = npkSensor.read();
        
        if (csvMode) {
            npkSensor.printCSV(data);
        } else {
            if (data.valid) {
                Serial.print("[");
                Serial.print(millis() / 1000);
                Serial.print("s] M:");
                Serial.print(data.moisture, 1);
                Serial.print("% T:");
                Serial.print(data.temperature, 1);
                Serial.print("C EC:");
                Serial.print(data.conductivity, 0);
                Serial.print(" pH:");
                Serial.print(data.ph, 2);
                Serial.print(" N:");
                Serial.print(data.nitrogen);
                Serial.print(" P:");
                Serial.print(data.phosphorus);
                Serial.print(" K:");
                Serial.println(data.potassium);
            } else {
                Serial.println("[ERROR] Failed to read NPK sensor - check wiring and power");
            }
        }
    }
}
