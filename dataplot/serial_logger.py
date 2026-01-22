#!/usr/bin/env python3
"""
================================================================================
SERIAL LOGGER - Autonomous Data Collection
================================================================================

This script automatically connects to an Arduino running the Outer Science Box
firmware, captures serial output, and saves it as CSV data with auto-generated
plots.

Operation Flow:
    1. Auto-detect Arduino serial port
    2. Connect and wait for Arduino to initialize
    3. Skip warmup/calibration messages
    4. Log CSV data for specified duration
    5. Auto-generate PNG plots
    6. Copy data to data.csv for plot_ppm.py compatibility

Usage:
    # Activate virtual environment first
    cd /Users/sarforajgazi/outer_science_box/dataplot
    source venv/bin/activate
    
    # Run with default duration (420 seconds = 7 minutes)
    python serial_logger.py
    
    # Run with custom duration (e.g., 300 seconds = 5 minutes)
    python serial_logger.py 300

Output Files:
    data_YYYYMMDD_HHMMSS.csv    - Timestamped raw data
    data.csv                    - Copy for plot_ppm.py
    mq_sensors_YYYYMMDD_HHMMSS.png    - MQ sensor plot
    environment_YYYYMMDD_HHMMSS.png   - BME280 plot

Dependencies:
    pip install pyserial pandas matplotlib

Author: Team Obseract
================================================================================
"""

import serial
import serial.tools.list_ports
import time
import sys
import os
from datetime import datetime

# ==============================================================================
# CONFIGURATION
# ==============================================================================
# These settings can be modified to change the logger behavior

BAUD_RATE = 9600              # Must match Arduino Serial.begin() rate
DURATION_SECONDS = 420        # Default logging duration (7 minutes)
                              # Can be overridden via command line argument

OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))  # Save files next to script
SKIP_WARMUP_LINES = True      # Skip warmup messages (Warming, Calibrating, etc.)

# ==============================================================================
# ARDUINO DETECTION
# ==============================================================================

def find_arduino_port():
    """
    Auto-detect the Arduino's serial port.
    
    Searches for common Arduino port names:
    - macOS: /dev/cu.usbmodem*, /dev/cu.usbserial*
    - Linux: /dev/ttyUSB*, /dev/ttyACM*
    
    Returns:
        str: Port path (e.g., "/dev/cu.usbmodem14201") or None if not found
    """
    # Get list of all available serial ports
    ports = serial.tools.list_ports.comports()
    
    # Search for Arduino-like port names
    for port in ports:
        # Common Arduino identifiers in port device names
        if 'usbmodem' in port.device or \
           'usbserial' in port.device or \
           'ttyUSB' in port.device or \
           'ttyACM' in port.device:
            return port.device
    
    # If no Arduino found, print available ports for debugging
    print("Available ports:")
    for port in ports:
        print(f"  {port.device} - {port.description}")
    
    return None


# ==============================================================================
# SERIAL DATA LOGGING
# ==============================================================================

def log_serial_data(port, duration, output_file):
    """
    Connect to Arduino and log serial data to CSV file.
    
    This function:
    1. Opens serial connection to Arduino
    2. Waits for Arduino reset (2 second delay)
    3. Filters out warmup/calibration messages
    4. Saves valid CSV lines to file
    5. Shows real-time progress
    
    Args:
        port: Serial port path (e.g., "/dev/cu.usbmodem14201")
        duration: Logging duration in seconds
        output_file: Path to save CSV data
    
    Returns:
        bool: True if data was logged successfully, False otherwise
    """
    print(f"Connecting to {port} at {BAUD_RATE} baud...")
    
    try:
        # Open serial connection
        # timeout=1 means readline() will wait max 1 second for data
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        
        # Arduino resets when serial connection opens
        # Wait 2 seconds for it to boot up
        time.sleep(2)
        
        print(f"Connected! Logging for {duration} seconds...")
        print(f"Output file: {output_file}")
        print("-" * 50)
        
        # Track timing and data count
        start_time = time.time()
        header_written = False
        line_count = 0
        
        # Open output file for writing
        with open(output_file, 'w') as f:
            # Write CSV header (matches Arduino output format)
            f.write("time_ms,site,sensor,value,unit,temp_C,hum_pct,press_hPa\n")
            
            # Main logging loop - runs until duration expires
            while (time.time() - start_time) < duration:
                
                # Check if data is available from Arduino
                if ser.in_waiting:
                    try:
                        # Read one line and decode from bytes to string
                        line = ser.readline().decode('utf-8').strip()
                        
                        # --------------------------------------------------
                        # FILTER: Skip warmup and calibration messages
                        # These are printed during Arduino startup
                        # --------------------------------------------------
                        if SKIP_WARMUP_LINES:
                            # Skip Arduino warmup messages
                            if 'Warming' in line or \
                               'remaining' in line or \
                               'Calibrat' in line or \
                               'complete' in line or \
                               'Ro:' in line:
                                print(f"[SKIP] {line}")
                                continue
                            
                            # Skip duplicate CSV header from Arduino
                            if 'time_ms' in line:
                                continue
                        
                        # --------------------------------------------------
                        # CHECK: Is this valid CSV data?
                        # Valid data lines start with a number (timestamp)
                        # --------------------------------------------------
                        if line and line[0].isdigit():
                            # Write data to file
                            f.write(line + '\n')
                            f.flush()  # Ensure data is written immediately
                            line_count += 1
                            
                            # Show progress with elapsed/remaining time
                            elapsed = int(time.time() - start_time)
                            remaining = duration - elapsed
                            # Truncate long lines for display
                            print(f"[{elapsed}s/{duration}s] Lines: {line_count} | {line[:60]}...")
                        else:
                            # Non-data line (info message from Arduino)
                            print(f"[INFO] {line}")
                            
                    except UnicodeDecodeError:
                        # Ignore corrupted characters
                        continue
                        
        # Logging complete
        print("-" * 50)
        print(f"Logging complete! Total lines: {line_count}")
        ser.close()
        
        return line_count > 0  # Return True if we got data
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        return False


# ==============================================================================
# PLOT GENERATION
# ==============================================================================

def generate_plots(csv_file, output_dir):
    """
    Generate and save PNG plots from collected CSV data.
    
    Creates two plot files:
    1. MQ gas sensors (2x2 grid): CH4, H2S, H2, CO2
    2. BME280 environment (1x3 grid): Temperature, Humidity, Pressure
    
    Args:
        csv_file: Path to CSV data file
        output_dir: Directory to save plot images
    
    Returns:
        bool: True if plots generated successfully
    """
    try:
        # Import plotting libraries (only when needed)
        import pandas as pd
        import matplotlib.pyplot as plt
        
        print("Generating plots...")
        
        # Load CSV data
        df = pd.read_csv(csv_file)
        df["time_s"] = df["time_ms"] / 1000.0  # Convert ms to seconds
        
        # Generate timestamp for filenames
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        # ==================================================================
        # PLOT 1: MQ Gas Sensors (2x2 grid)
        # ==================================================================
        sensors = [
            ("MQ4_CH4",   "MQ-4 Methane (CH₄)",   "tab:blue"),
            ("MQ136_H2S", "MQ-136 H₂S",           "tab:orange"),
            ("MQ8_H2",    "MQ-8 Hydrogen (H₂)",   "tab:green"),
            ("MQ135_CO2", "MQ-135 CO₂",           "tab:red")
        ]
        
        fig, axes = plt.subplots(2, 2, figsize=(14, 10))
        axes = axes.flatten()
        
        for ax, (sensor_name, title, color) in zip(axes, sensors):
            data = df[df["sensor"] == sensor_name]
            if not data.empty:
                ax.plot(data["time_s"], data["value"], color=color, linewidth=1.5)
                ax.set_title(title, fontsize=12)
                ax.set_xlabel("Time (s)")
                ax.set_ylabel(f"{data['unit'].iloc[0]}")
                ax.grid(True, alpha=0.3)
            else:
                ax.set_title(f"{title}\n(No Data)")
        
        plt.suptitle("MQ Gas Sensors — Team Obseract Rover", fontsize=16)
        plt.tight_layout(rect=[0, 0, 1, 0.96])
        
        # Save MQ sensors plot
        mq_plot_file = os.path.join(output_dir, f"mq_sensors_{timestamp}.png")
        plt.savefig(mq_plot_file, dpi=150)  # 150 DPI for good quality
        print(f"Saved: {mq_plot_file}")
        plt.close()
        
        # ==================================================================
        # PLOT 2: BME280 Environment (1x3 grid)
        # ==================================================================
        env_params = [
            ("BME_TEMP",  "Temperature", "°C",  "tab:red"),
            ("BME_HUM",   "Humidity",    "%",   "tab:blue"),
            ("BME_PRESS", "Pressure",    "hPa", "tab:green")
        ]
        
        fig, axes = plt.subplots(1, 3, figsize=(15, 4))
        
        for ax, (sensor_name, title, unit, color) in zip(axes, env_params):
            data = df[df["sensor"] == sensor_name]
            if not data.empty:
                ax.plot(data["time_s"], data["value"], color=color, linewidth=1.5)
                ax.set_title(title, fontsize=12)
                ax.set_xlabel("Time (s)")
                ax.set_ylabel(unit)
                ax.grid(True, alpha=0.3)
        
        plt.suptitle("BME280 Environmental Data — Team Obseract Rover", fontsize=14)
        plt.tight_layout(rect=[0, 0, 1, 0.92])
        
        # Save environment plot
        env_plot_file = os.path.join(output_dir, f"environment_{timestamp}.png")
        plt.savefig(env_plot_file, dpi=150)
        print(f"Saved: {env_plot_file}")
        plt.close()
        
        return True
        
    except Exception as e:
        print(f"Error generating plots: {e}")
        return False


# ==============================================================================
# MAIN FUNCTION
# ==============================================================================

def main():
    """
    Main entry point for the serial logger.
    
    Workflow:
    1. Parse command line arguments for duration
    2. Auto-detect Arduino
    3. Log data for specified duration
    4. Generate plots
    5. Copy data to data.csv
    """
    # Print banner
    print("=" * 50)
    print("  MQ Sensor Autonomous Data Logger")
    print("  Team Obseract Rover")
    print("=" * 50)
    
    # --------------------------------------------------
    # Parse command line arguments
    # Usage: python serial_logger.py [duration_seconds]
    # --------------------------------------------------
    duration = DURATION_SECONDS  # Use default from config
    
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print(f"Usage: python serial_logger.py [duration_seconds]")
            sys.exit(1)
    
    print(f"Duration: {duration} seconds ({duration/60:.1f} minutes)")
    
    # --------------------------------------------------
    # Find Arduino
    # --------------------------------------------------
    port = find_arduino_port()
    if not port:
        print("Error: No Arduino found!")
        print("Make sure Arduino is connected via USB")
        sys.exit(1)
    
    print(f"Found Arduino at: {port}")
    
    # --------------------------------------------------
    # Generate output filename with timestamp
    # Format: data_20260122_130559.csv
    # --------------------------------------------------
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_file = os.path.join(OUTPUT_DIR, f"data_{timestamp}.csv")
    
    # --------------------------------------------------
    # Log data from Arduino
    # --------------------------------------------------
    success = log_serial_data(port, duration, csv_file)
    
    if success:
        # Generate plots from collected data
        generate_plots(csv_file, OUTPUT_DIR)
        
        # Copy to data.csv for compatibility with plot_ppm.py
        import shutil
        data_csv = os.path.join(OUTPUT_DIR, "data.csv")
        shutil.copy(csv_file, data_csv)
        print(f"\nAlso copied to: data.csv")
        
        # Success message
        print("\n" + "=" * 50)
        print("  COMPLETE!")
        print("=" * 50)
    else:
        print("No data logged.")
        print("Check Arduino connection and ensure firmware is uploaded.")


# ==============================================================================
# SCRIPT ENTRY POINT
# ==============================================================================

if __name__ == "__main__":
    main()
