/**
 * =============================================================================
 * NPK 7-in-1 SOIL SENSOR - RS485 MODBUS RTU (Arduino Uno)
 * =============================================================================
 * 
 * Reads 7 parameters: Moisture, Temperature, EC, pH, Nitrogen, Phosphorus, Potassium
 * 
 * WIRING (Arduino Uno + RS485 Module):
 *   RS485 DI  -> Pin 3 (SoftwareSerial TX)
 *   RS485 RO  -> Pin 2 (SoftwareSerial RX)
 *   RS485 DE  -> Pin 7 (or connect DE+RE to Pin 7)
 *   RS485 RE  -> Pin 7 (connect together with DE)
 *   RS485 VCC -> 5V
 *   RS485 GND -> GND
 *   
 *   NPK Sensor A+ -> RS485 A+
 *   NPK Sensor B- -> RS485 B-
 *   NPK Sensor VCC -> 12V-24V DC (external power supply)
 *   NPK Sensor GND -> GND (common ground with Arduino)
 * 
 * Upload: Select "Arduino Uno" and upload
 * Monitor: Tools > Serial Monitor @ 9600 baud
 * =============================================================================
 */

#include <SoftwareSerial.h>

// =============================================================================
// CONFIGURATION - ADJUST THESE FOR YOUR SETUP
// =============================================================================

#define RS485_RX_PIN    2     // Connect to RS485 RO
#define RS485_TX_PIN    3     // Connect to RS485 DI
#define RS485_DE_PIN    7     // Connect to RS485 DE and RE

#define NPK_SLAVE_ADDR  0x01  // Sensor address (try 0x01, 0x02, or 0xFF)
#define NPK_BAUD_RATE   4800  // Common: 4800, 9600, 19200

#define READ_INTERVAL   2000  // Read every 2 seconds

// =============================================================================
// GLOBALS
// =============================================================================

SoftwareSerial rs485(RS485_RX_PIN, RS485_TX_PIN);  // RX, TX

// Modbus query: Read 7 registers starting from 0x0000
byte queryFrame[8];
byte responseBuffer[25];

unsigned long lastReadTime = 0;

// =============================================================================
// MODBUS CRC-16 CALCULATION
// =============================================================================

uint16_t calculateCRC(byte* data, byte length) {
    uint16_t crc = 0xFFFF;
    for (byte i = 0; i < length; i++) {
        crc ^= data[i];
        for (byte j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// =============================================================================
// BUILD MODBUS QUERY FRAME
// =============================================================================

void buildQueryFrame() {
    queryFrame[0] = NPK_SLAVE_ADDR;    // Slave Address
    queryFrame[1] = 0x03;              // Function Code: Read Holding Registers
    queryFrame[2] = 0x00;              // Start Register High
    queryFrame[3] = 0x00;              // Start Register Low
    queryFrame[4] = 0x00;              // Number of Registers High
    queryFrame[5] = 0x07;              // Number of Registers Low (7 registers)
    
    uint16_t crc = calculateCRC(queryFrame, 6);
    queryFrame[6] = crc & 0xFF;        // CRC Low
    queryFrame[7] = (crc >> 8) & 0xFF; // CRC High
}

// =============================================================================
// READ NPK SENSOR
// =============================================================================

bool readNPKSensor(float &moisture, float &temperature, float &ec, float &ph,
                   uint16_t &nitrogen, uint16_t &phosphorus, uint16_t &potassium) {
    
    // Clear buffer
    while (rs485.available()) {
        rs485.read();
    }
    
    // Set to transmit mode
    digitalWrite(RS485_DE_PIN, HIGH);
    delay(1);
    
    // Send query
    for (int i = 0; i < 8; i++) {
        rs485.write(queryFrame[i]);
    }
    rs485.flush();
    
    // Set to receive mode
    delay(1);
    digitalWrite(RS485_DE_PIN, LOW);
    
    // Wait for response (19 bytes expected)
    byte expectedLen = 19;  // 3 header + 14 data + 2 CRC
    byte index = 0;
    unsigned long startTime = millis();
    
    while ((millis() - startTime) < 500 && index < expectedLen) {
        if (rs485.available()) {
            responseBuffer[index++] = rs485.read();
        }
    }
    
    // Validate response length
    if (index < expectedLen) {
        return false;
    }
    
    // Validate slave address and function code
    if (responseBuffer[0] != NPK_SLAVE_ADDR || responseBuffer[1] != 0x03) {
        return false;
    }
    
    // Validate byte count
    if (responseBuffer[2] != 14) {  // 7 registers * 2 bytes
        return false;
    }
    
    // Verify CRC
    uint16_t receivedCRC = responseBuffer[17] | (responseBuffer[18] << 8);
    uint16_t calculatedCRC = calculateCRC(responseBuffer, 17);
    if (receivedCRC != calculatedCRC) {
        return false;
    }
    
    // Parse data (registers start at index 3)
    uint16_t rawMoisture    = (responseBuffer[3] << 8)  | responseBuffer[4];
    uint16_t rawTemperature = (responseBuffer[5] << 8)  | responseBuffer[6];
    uint16_t rawEC          = (responseBuffer[7] << 8)  | responseBuffer[8];
    uint16_t rawPH          = (responseBuffer[9] << 8)  | responseBuffer[10];
    nitrogen                = (responseBuffer[11] << 8) | responseBuffer[12];
    phosphorus              = (responseBuffer[13] << 8) | responseBuffer[14];
    potassium               = (responseBuffer[15] << 8) | responseBuffer[16];
    
    // Convert values (most sensors use /10 scaling)
    moisture    = rawMoisture / 10.0;
    temperature = rawTemperature / 10.0;
    ec          = (float)rawEC;           // µS/cm, usually no scaling
    ph          = rawPH / 10.0;
    
    return true;
}

// =============================================================================
// SETUP
// =============================================================================

void setup() {
    // Initialize debug serial
    Serial.begin(9600);
    Serial.println();
    Serial.println(F("================================"));
    Serial.println(F("  NPK 7-in-1 Sensor (RS485)"));
    Serial.println(F("================================"));
    Serial.println();
    
    // Initialize RS485 control pin
    pinMode(RS485_DE_PIN, OUTPUT);
    digitalWrite(RS485_DE_PIN, LOW);  // Start in receive mode
    
    // Initialize SoftwareSerial for RS485
    rs485.begin(NPK_BAUD_RATE);
    
    // Build the query frame
    buildQueryFrame();
    
    Serial.println(F("Wiring:"));
    Serial.println(F("  RS485 RO  -> Pin 2"));
    Serial.println(F("  RS485 DI  -> Pin 3"));
    Serial.println(F("  RS485 DE+RE -> Pin 7"));
    Serial.println();
    Serial.print(F("Baud: ")); Serial.println(NPK_BAUD_RATE);
    Serial.print(F("Addr: 0x")); Serial.println(NPK_SLAVE_ADDR, HEX);
    Serial.println();
    Serial.println(F("Reading sensor every 2 seconds..."));
    Serial.println();
    
    // Print CSV header
    Serial.println(F("Moisture(%),Temp(C),EC(uS/cm),pH,N(mg/kg),P(mg/kg),K(mg/kg)"));
}

// =============================================================================
// MAIN LOOP
// =============================================================================

void loop() {
    if (millis() - lastReadTime >= READ_INTERVAL) {
        lastReadTime = millis();
        
        float moisture, temperature, ec, ph;
        uint16_t nitrogen, phosphorus, potassium;
        
        bool success = readNPKSensor(moisture, temperature, ec, ph, 
                                      nitrogen, phosphorus, potassium);
        
        if (success) {
            // Print as CSV
            Serial.print(moisture, 1);     Serial.print(",");
            Serial.print(temperature, 1);  Serial.print(",");
            Serial.print(ec, 0);           Serial.print(",");
            Serial.print(ph, 2);           Serial.print(",");
            Serial.print(nitrogen);        Serial.print(",");
            Serial.print(phosphorus);      Serial.print(",");
            Serial.println(potassium);
        } else {
            Serial.println(F("ERROR: No response - check wiring, power, address"));
        }
    }
}
