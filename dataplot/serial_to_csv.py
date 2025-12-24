import serial
import csv
import os

# 1. Configuration
# Change 'COM3' to the port your Arduino Mega is connected to (e.g., '/dev/ttyUSB0' on Linux)
PORT = 'COM3' 
BAUD_RATE = 9600
# Save the file in the same folder as this script
FILE_NAME = os.path.join(os.path.dirname(__file__), "data.csv")

try:
    # 2. Open Serial Port
    ser = serial.Serial(PORT, BAUD_RATE, timeout=1)
    print(f"Connected to {PORT}. Logging data to {FILE_NAME}...")

    # 3. Open CSV file in 'append' mode
    with open(FILE_NAME, "a", newline="") as f:
        writer = csv.writer(f)
        
        while True:
            # Read a line from Arduino
            line = ser.readline().decode('utf-8').strip()
            
            if line:
                # Print to console so you can see it live
                print(f"Received: {line}")
                
                # Split by comma and write to CSV
                data_row = line.split(",")
                writer.writerow(data_row)
                f.flush() # Ensure data is saved immediately

except KeyboardInterrupt:
    print("\nLogging stopped by user.")
except Exception as e:
    print(f"Error: {e}")
    