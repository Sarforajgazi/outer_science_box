/**
 * 7-in-1 NPK Soil Sensor (RS485 Modbus)
 * Arduino Mega Pro
 * 
 * Wiring:
 *   RO  → RX1 (pin 19)
 *   DI  → TX1 (pin 18)
 *   RE  → pin 6
 *   DE  → pin 7
 *   VCC → 5V (module) | Sensor needs 12-24V DC
 *   GND → GND
 */

#include <Arduino.h>

#define RE 6
#define DE 7
#define NPK_SERIAL Serial1
#define BAUD_RATE 4800  // Try 9600, 4800, 2400 if issues persist

byte request[8];
byte response[19];

// ─────────────────────────────────────────────────────────────
// CRC16 Modbus
// ─────────────────────────────────────────────────────────────
uint16_t modbusCRC(byte *buf, int len) {
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buf[pos];
        for (int i = 8; i != 0; i--) {
            if (crc & 0x0001) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ─────────────────────────────────────────────────────────────
// Send Modbus Request
// ─────────────────────────────────────────────────────────────
void sendRequest() {
    request[0] = 0x01;  // Slave address
    request[1] = 0x03;  // Function code: Read Holding Registers
    request[2] = 0x00;  // Start address high
    request[3] = 0x00;  // Start address low
    request[4] = 0x00;  // Number of registers high
    request[5] = 0x07;  // Number of registers low (7 registers)

    uint16_t crc = modbusCRC(request, 6);
    request[6] = crc & 0xFF;        // CRC low byte
    request[7] = (crc >> 8) & 0xFF; // CRC high byte

    // Enable transmit mode
    digitalWrite(DE, HIGH);
    digitalWrite(RE, HIGH);
    delay(10);

    NPK_SERIAL.write(request, 8);
    NPK_SERIAL.flush();

    // Switch to receive mode
    digitalWrite(DE, LOW);
    digitalWrite(RE, LOW);
}

// ─────────────────────────────────────────────────────────────
// Validate Response
// ─────────────────────────────────────────────────────────────
bool validateResponse() {
    // Check header: Address, Function, Byte Count
    if (response[0] != 0x01) {
        Serial.println(F("ERROR: Wrong slave address"));
        return false;
    }
    if (response[1] != 0x03) {
        Serial.println(F("ERROR: Wrong function code"));
        return false;
    }
    if (response[2] != 0x0E) {  // 14 bytes = 7 registers × 2 bytes
        Serial.println(F("ERROR: Wrong byte count"));
        return false;
    }

    // Validate CRC (bytes 17-18 contain CRC)
    uint16_t receivedCRC = (response[18] << 8) | response[17];
    uint16_t calculatedCRC = modbusCRC(response, 17);

    if (receivedCRC != calculatedCRC) {
        Serial.print(F("ERROR: CRC mismatch! Received: 0x"));
        Serial.print(receivedCRC, HEX);
        Serial.print(F(" Calculated: 0x"));
        Serial.println(calculatedCRC, HEX);
        return false;
    }

    return true;
}

// ─────────────────────────────────────────────────────────────
// Print Raw Response (for debugging)
// ─────────────────────────────────────────────────────────────
void printRawResponse() {
    Serial.print(F("RAW: "));
    for (int i = 0; i < 19; i++) {
        if (response[i] < 0x10) Serial.print("0");
        Serial.print(response[i], HEX);
        Serial.print(" ");
    }
    Serial.println();
}

// ─────────────────────────────────────────────────────────────
// Parse and Print Sensor Data
// ─────────────────────────────────────────────────────────────
void parseSensorData() {
    // Data starts at response[3]
    // Each register is 2 bytes (high byte first)

    float moisture    = ((response[3] << 8) | response[4]) / 10.0;       // %
    int16_t tempRaw   = (response[5] << 8) | response[6];                // Signed for negative temps
    float temperature = tempRaw / 10.0;                                   // °C
    uint16_t ec       = (response[7] << 8) | response[8];                // µS/cm
    float ph          = ((response[9] << 8) | response[10]) / 10.0;      // pH
    uint16_t nitrogen = (response[11] << 8) | response[12];              // mg/kg
    uint16_t phosphorus = (response[13] << 8) | response[14];            // mg/kg
    uint16_t potassium = (response[15] << 8) | response[16];             // mg/kg

    // Sanity checks (typical valid ranges)
    bool valid = true;
    if (moisture < 0 || moisture > 100) {
        Serial.println(F("WARNING: Moisture out of range (0-100%)"));
        valid = false;
    }
    if (temperature < -40 || temperature > 80) {
        Serial.println(F("WARNING: Temperature out of range (-40 to 80°C)"));
        valid = false;
    }
    if (ph < 0 || ph > 14) {
        Serial.println(F("WARNING: pH out of range (0-14)"));
        valid = false;
    }

    Serial.println(F("─────────────────────────"));
    Serial.print(F("Moisture:    ")); Serial.print(moisture, 1);    Serial.println(F(" %"));
    Serial.print(F("Temperature: ")); Serial.print(temperature, 1); Serial.println(F(" °C"));
    Serial.print(F("EC:          ")); Serial.print(ec);             Serial.println(F(" µS/cm"));
    Serial.print(F("pH:          ")); Serial.print(ph, 1);          Serial.println();
    Serial.print(F("Nitrogen:    ")); Serial.print(nitrogen);       Serial.println(F(" mg/kg"));
    Serial.print(F("Phosphorus:  ")); Serial.print(phosphorus);     Serial.println(F(" mg/kg"));
    Serial.print(F("Potassium:   ")); Serial.print(potassium);      Serial.println(F(" mg/kg"));
    
    if (!valid) {
        Serial.println(F("⚠️  Some values out of expected range!"));
    }
    Serial.println(F("─────────────────────────"));
}

// ─────────────────────────────────────────────────────────────
// Setup
// ─────────────────────────────────────────────────────────────
void setup() {
    Serial.begin(9600);
    while (!Serial && millis() < 2000) {
        delay(10);
    }

    NPK_SERIAL.begin(BAUD_RATE);

    pinMode(RE, OUTPUT);
    pinMode(DE, OUTPUT);
    digitalWrite(RE, LOW);
    digitalWrite(DE, LOW);

    Serial.println(F("═══════════════════════════════════"));
    Serial.println(F("  7-in-1 NPK Sensor (RS485 Modbus)"));
    Serial.print(F("  Baud Rate: ")); Serial.println(BAUD_RATE);
    Serial.println(F("═══════════════════════════════════"));
}

// ─────────────────────────────────────────────────────────────
// Main Loop
// ─────────────────────────────────────────────────────────────
void loop() {
    // Clear any stale data in buffer
    while (NPK_SERIAL.available()) {
        NPK_SERIAL.read();
    }

    sendRequest();
    delay(300);  // Wait for sensor response

    int bytesAvailable = NPK_SERIAL.available();

    if (bytesAvailable >= 19) {
        for (int i = 0; i < 19; i++) {
            response[i] = NPK_SERIAL.read();
        }

        // Debug: Print raw bytes
        printRawResponse();

        if (validateResponse()) {
            parseSensorData();
        } else {
            Serial.println(F("Data validation failed - check wiring/baud rate"));
        }

    } else if (bytesAvailable > 0) {
        Serial.print(F("Incomplete response: "));
        Serial.print(bytesAvailable);
        Serial.println(F(" bytes (expected 19)"));
        
        // Print what we got
        Serial.print(F("RAW: "));
        while (NPK_SERIAL.available()) {
            byte b = NPK_SERIAL.read();
            if (b < 0x10) Serial.print("0");
            Serial.print(b, HEX);
            Serial.print(" ");
        }
        Serial.println();
        
    } else {
        Serial.println(F("No response from sensor"));
        Serial.println(F("Check: Power (12-24V), Wiring (A/B), Baud rate"));
    }

    delay(2000);
}
