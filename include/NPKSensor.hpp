/**
 * =============================================================================
 * NPK 7-in-1 SOIL SENSOR - RS485 MODBUS RTU DRIVER
 * =============================================================================
 * 
 * This library reads data from a 7-in-1 NPK soil sensor via RS485 Modbus RTU.
 * 
 * Measured Parameters:
 *   1. Moisture (%)
 *   2. Temperature (°C)
 *   3. Electrical Conductivity (µS/cm)
 *   4. pH
 *   5. Nitrogen (mg/kg)
 *   6. Phosphorus (mg/kg)
 *   7. Potassium (mg/kg)
 * 
 * Hardware Connections (Arduino Mega):
 *   - RS485 Module DI (Data In)  -> TX1 (Pin 18) or TX2 (Pin 16) or TX3 (Pin 14)
 *   - RS485 Module RO (Data Out) -> RX1 (Pin 19) or RX2 (Pin 17) or RX3 (Pin 15)
 *   - RS485 Module DE + RE       -> Control Pin (any digital pin, e.g., Pin 8)
 *   - RS485 Module VCC           -> 5V
 *   - RS485 Module GND           -> GND
 *   - NPK Sensor A+ (Yellow)     -> RS485 A+
 *   - NPK Sensor B- (White)      -> RS485 B-
 *   - NPK Sensor VCC (Red)       -> 12V-24V DC (check sensor specs)
 *   - NPK Sensor GND (Black)     -> GND (common ground)
 * 
 * Author: Team Obseract
 * =============================================================================
 */

#ifndef NPK_SENSOR_HPP
#define NPK_SENSOR_HPP

#include <Arduino.h>

// Default Modbus settings for most 7-in-1 NPK sensors
#define NPK_DEFAULT_BAUD_RATE     4800
#define NPK_DEFAULT_SLAVE_ADDR    0x01
#define NPK_READ_FUNCTION_CODE    0x03
#define NPK_START_REGISTER        0x0000
#define NPK_NUM_REGISTERS         7

// Response timeout in milliseconds
#define NPK_RESPONSE_TIMEOUT      500

// Data structure to hold all sensor readings
struct NPKData {
    float moisture;       // % (0-100)
    float temperature;    // °C
    float conductivity;   // µS/cm (Electrical Conductivity)
    float ph;             // pH (0-14)
    uint16_t nitrogen;    // mg/kg (N)
    uint16_t phosphorus;  // mg/kg (P)
    uint16_t potassium;   // mg/kg (K)
    bool valid;           // Data validity flag
};

class NPKSensor {
private:
    HardwareSerial* _serial;
    uint8_t _dePin;           // DE/RE control pin for RS485 direction
    uint8_t _slaveAddr;       // Modbus slave address
    uint32_t _baudRate;
    
    // Query frame for reading 7 registers starting from 0x0000
    // Structure: [Slave Addr][Function Code][Start Addr High][Start Addr Low][Num Registers High][Num Registers Low][CRC Low][CRC High]
    uint8_t _queryFrame[8];
    
    // Response buffer (max: 3 + 2*7 + 2 = 19 bytes)
    uint8_t _responseBuffer[25];
    
    /**
     * Calculate Modbus CRC-16
     */
    uint16_t calculateCRC(uint8_t* data, uint8_t length) {
        uint16_t crc = 0xFFFF;
        for (uint8_t i = 0; i < length; i++) {
            crc ^= data[i];
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x0001) {
                    crc = (crc >> 1) ^ 0xA001;
                } else {
                    crc >>= 1;
                }
            }
        }
        return crc;
    }
    
    /**
     * Build the Modbus query frame
     */
    void buildQueryFrame() {
        _queryFrame[0] = _slaveAddr;                    // Slave Address
        _queryFrame[1] = NPK_READ_FUNCTION_CODE;        // Function Code (0x03 = Read Holding Registers)
        _queryFrame[2] = (NPK_START_REGISTER >> 8);     // Start Register High Byte
        _queryFrame[3] = (NPK_START_REGISTER & 0xFF);   // Start Register Low Byte
        _queryFrame[4] = (NPK_NUM_REGISTERS >> 8);      // Number of Registers High Byte
        _queryFrame[5] = (NPK_NUM_REGISTERS & 0xFF);    // Number of Registers Low Byte
        
        uint16_t crc = calculateCRC(_queryFrame, 6);
        _queryFrame[6] = crc & 0xFF;                    // CRC Low Byte
        _queryFrame[7] = (crc >> 8) & 0xFF;             // CRC High Byte
    }
    
    /**
     * Set RS485 transceiver to transmit mode
     */
    void setTransmitMode() {
        digitalWrite(_dePin, HIGH);
        delayMicroseconds(50);
    }
    
    /**
     * Set RS485 transceiver to receive mode
     */
    void setReceiveMode() {
        delayMicroseconds(50);
        digitalWrite(_dePin, LOW);
    }
    
    /**
     * Clear the serial receive buffer
     */
    void clearBuffer() {
        while (_serial->available()) {
            _serial->read();
        }
    }

public:
    /**
     * Constructor
     * @param serial Pointer to HardwareSerial (Serial1, Serial2, or Serial3)
     * @param dePin DE/RE control pin for RS485 direction control
     * @param slaveAddr Modbus slave address (default: 0x01)
     * @param baudRate Serial baud rate (default: 4800)
     */
    NPKSensor(HardwareSerial* serial, uint8_t dePin, 
              uint8_t slaveAddr = NPK_DEFAULT_SLAVE_ADDR, 
              uint32_t baudRate = NPK_DEFAULT_BAUD_RATE) {
        _serial = serial;
        _dePin = dePin;
        _slaveAddr = slaveAddr;
        _baudRate = baudRate;
    }
    
    /**
     * Initialize the sensor
     */
    void begin() {
        pinMode(_dePin, OUTPUT);
        digitalWrite(_dePin, LOW);  // Start in receive mode
        
        _serial->begin(_baudRate);
        
        // Build the query frame
        buildQueryFrame();
        
        delay(100);  // Allow sensor to stabilize
    }
    
    /**
     * Read all sensor data
     * @return NPKData structure with all readings
     */
    NPKData read() {
        NPKData data;
        data.valid = false;
        data.moisture = 0;
        data.temperature = 0;
        data.conductivity = 0;
        data.ph = 0;
        data.nitrogen = 0;
        data.phosphorus = 0;
        data.potassium = 0;
        
        // Clear any pending data
        clearBuffer();
        
        // Send query
        setTransmitMode();
        _serial->write(_queryFrame, 8);
        _serial->flush();  // Wait for transmission to complete
        setReceiveMode();
        
        // Wait for response with timeout
        unsigned long startTime = millis();
        uint8_t responseIndex = 0;
        uint8_t expectedLength = 3 + (NPK_NUM_REGISTERS * 2) + 2;  // Header(3) + Data(14) + CRC(2) = 19 bytes
        
        while ((millis() - startTime) < NPK_RESPONSE_TIMEOUT) {
            if (_serial->available()) {
                _responseBuffer[responseIndex++] = _serial->read();
                
                if (responseIndex >= expectedLength) {
                    break;
                }
            }
        }
        
        // Validate response
        if (responseIndex < expectedLength) {
            return data;  // Incomplete response
        }
        
        // Check slave address and function code
        if (_responseBuffer[0] != _slaveAddr || _responseBuffer[1] != NPK_READ_FUNCTION_CODE) {
            return data;  // Invalid response
        }
        
        // Check byte count
        if (_responseBuffer[2] != (NPK_NUM_REGISTERS * 2)) {
            return data;  // Invalid byte count
        }
        
        // Verify CRC
        uint16_t receivedCRC = _responseBuffer[expectedLength - 2] | (_responseBuffer[expectedLength - 1] << 8);
        uint16_t calculatedCRC = calculateCRC(_responseBuffer, expectedLength - 2);
        
        if (receivedCRC != calculatedCRC) {
            return data;  // CRC mismatch
        }
        
        // Parse data (registers start at index 3)
        // Register order depends on sensor model. Common order:
        // Moisture, Temperature, EC, pH, Nitrogen, Phosphorus, Potassium
        
        uint16_t rawMoisture    = (_responseBuffer[3] << 8)  | _responseBuffer[4];
        uint16_t rawTemperature = (_responseBuffer[5] << 8)  | _responseBuffer[6];
        uint16_t rawEC          = (_responseBuffer[7] << 8)  | _responseBuffer[8];
        uint16_t rawPH          = (_responseBuffer[9] << 8)  | _responseBuffer[10];
        uint16_t rawNitrogen    = (_responseBuffer[11] << 8) | _responseBuffer[12];
        uint16_t rawPhosphorus  = (_responseBuffer[13] << 8) | _responseBuffer[14];
        uint16_t rawPotassium   = (_responseBuffer[15] << 8) | _responseBuffer[16];
        
        // Convert raw values (scaling depends on sensor model)
        // Most sensors: moisture/10, temperature/10, pH/10
        data.moisture     = rawMoisture / 10.0;
        data.temperature  = rawTemperature / 10.0;
        data.conductivity = (float)rawEC;                 // µS/cm (usually no scaling)
        data.ph           = rawPH / 10.0;
        data.nitrogen     = rawNitrogen;                  // mg/kg
        data.phosphorus   = rawPhosphorus;                // mg/kg
        data.potassium    = rawPotassium;                 // mg/kg
        
        data.valid = true;
        return data;
    }
    
    /**
     * Print sensor data to Serial (for debugging)
     */
    void printData(NPKData& data) {
        if (!data.valid) {
            Serial.println("NPK Sensor: Invalid data or communication error");
            return;
        }
        
        Serial.println("========== NPK 7-in-1 Sensor Data ==========");
        Serial.print("Moisture:     "); Serial.print(data.moisture, 1);     Serial.println(" %");
        Serial.print("Temperature:  "); Serial.print(data.temperature, 1);  Serial.println(" °C");
        Serial.print("EC:           "); Serial.print(data.conductivity, 0); Serial.println(" µS/cm");
        Serial.print("pH:           "); Serial.print(data.ph, 2);           Serial.println("");
        Serial.print("Nitrogen:     "); Serial.print(data.nitrogen);        Serial.println(" mg/kg");
        Serial.print("Phosphorus:   "); Serial.print(data.phosphorus);      Serial.println(" mg/kg");
        Serial.print("Potassium:    "); Serial.print(data.potassium);       Serial.println(" mg/kg");
        Serial.println("=============================================");
    }
    
    /**
     * Print CSV header
     */
    void printCSVHeader() {
        Serial.println("Moisture(%),Temperature(C),EC(uS/cm),pH,Nitrogen(mg/kg),Phosphorus(mg/kg),Potassium(mg/kg)");
    }
    
    /**
     * Print data as CSV row
     */
    void printCSV(NPKData& data) {
        if (!data.valid) {
            Serial.println("ERROR,ERROR,ERROR,ERROR,ERROR,ERROR,ERROR");
            return;
        }
        
        Serial.print(data.moisture, 1);     Serial.print(",");
        Serial.print(data.temperature, 1);  Serial.print(",");
        Serial.print(data.conductivity, 0); Serial.print(",");
        Serial.print(data.ph, 2);           Serial.print(",");
        Serial.print(data.nitrogen);        Serial.print(",");
        Serial.print(data.phosphorus);      Serial.print(",");
        Serial.println(data.potassium);
    }
    
    /**
     * Change slave address (for advanced use)
     */
    void setSlaveAddress(uint8_t addr) {
        _slaveAddr = addr;
        buildQueryFrame();
    }
    
    /**
     * Get current slave address
     */
    uint8_t getSlaveAddress() {
        return _slaveAddr;
    }
};

#endif // NPK_SENSOR_HPP
