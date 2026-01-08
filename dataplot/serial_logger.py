#!/usr/bin/env python3
"""
Autonomous Serial Data Logger for MQ Sensors
- Connects to Arduino serial port
- Logs CSV data for specified duration
- Auto-generates and saves plots
"""

import serial
import serial.tools.list_ports
import time
import sys
import os
from datetime import datetime

# ============ CONFIGURATION ============
BAUD_RATE = 9600
DURATION_SECONDS = 300  # 5 minutes default (change as needed)
OUTPUT_DIR = os.path.dirname(os.path.abspath(__file__))
SKIP_WARMUP_LINES = True  # Skip lines until CSV header detected
# =======================================

def find_arduino_port():
    """Auto-detect Arduino port"""
    ports = serial.tools.list_ports.comports()
    for port in ports:
        if 'usbmodem' in port.device or 'usbserial' in port.device or 'ttyUSB' in port.device or 'ttyACM' in port.device:
            return port.device
    # Fallback: list all ports
    print("Available ports:")
    for port in ports:
        print(f"  {port.device} - {port.description}")
    return None

def log_serial_data(port, duration, output_file):
    """Log serial data to CSV for specified duration"""
    print(f"Connecting to {port} at {BAUD_RATE} baud...")
    
    try:
        ser = serial.Serial(port, BAUD_RATE, timeout=1)
        time.sleep(2)  # Wait for Arduino reset
        
        print(f"Connected! Logging for {duration} seconds...")
        print(f"Output file: {output_file}")
        print("-" * 50)
        
        start_time = time.time()
        header_written = False
        line_count = 0
        
        with open(output_file, 'w') as f:
            # Write CSV header
            f.write("time_ms,site,sensor,value,unit,temp_C,hum_pct,press_hPa\n")
            
            while (time.time() - start_time) < duration:
                if ser.in_waiting:
                    try:
                        line = ser.readline().decode('utf-8').strip()
                        
                        # Skip warmup messages
                        if SKIP_WARMUP_LINES:
                            if 'Warming' in line or 'remaining' in line or 'Calibrat' in line or 'complete' in line or 'Ro:' in line:
                                print(f"[SKIP] {line}")
                                continue
                            if 'time_ms' in line:  # Skip CSV header from Arduino
                                continue
                        
                        # Check if valid CSV data (starts with number)
                        if line and line[0].isdigit():
                            f.write(line + '\n')
                            f.flush()
                            line_count += 1
                            
                            # Show progress
                            elapsed = int(time.time() - start_time)
                            remaining = duration - elapsed
                            print(f"[{elapsed}s/{duration}s] Lines: {line_count} | {line[:60]}...")
                        else:
                            print(f"[INFO] {line}")
                            
                    except UnicodeDecodeError:
                        continue
                        
        print("-" * 50)
        print(f"Logging complete! Total lines: {line_count}")
        ser.close()
        return line_count > 0
        
    except serial.SerialException as e:
        print(f"Error: {e}")
        return False

def generate_plots(csv_file, output_dir):
    """Generate and save plots from CSV data"""
    try:
        import pandas as pd
        import matplotlib.pyplot as plt
        
        print("Generating plots...")
        
        df = pd.read_csv(csv_file)
        df["time_s"] = df["time_ms"] / 1000.0
        
        timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
        
        # ===== MQ Sensors Plot =====
        sensors = [
            ("MQ4_CH4", "MQ-4 Methane (CH₄)", "tab:blue"),
            ("MQ136_H2S", "MQ-136 H₂S", "tab:orange"),
            ("MQ8_H2", "MQ-8 Hydrogen (H₂)", "tab:green"),
            ("MQ135_CO2", "MQ-135 CO₂", "tab:red")
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
        
        mq_plot_file = os.path.join(output_dir, f"mq_sensors_{timestamp}.png")
        plt.savefig(mq_plot_file, dpi=150)
        print(f"Saved: {mq_plot_file}")
        plt.close()
        
        # ===== BME280 Environmental Plot =====
        env_params = [
            ("BME_TEMP", "Temperature", "°C", "tab:red"),
            ("BME_HUM", "Humidity", "%", "tab:blue"),
            ("BME_PRESS", "Pressure", "hPa", "tab:green")
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
        
        env_plot_file = os.path.join(output_dir, f"environment_{timestamp}.png")
        plt.savefig(env_plot_file, dpi=150)
        print(f"Saved: {env_plot_file}")
        plt.close()
        
        return True
        
    except Exception as e:
        print(f"Error generating plots: {e}")
        return False

def main():
    print("=" * 50)
    print("  MQ Sensor Autonomous Data Logger")
    print("  Team Obseract Rover")
    print("=" * 50)
    
    # Parse command line args
    duration = DURATION_SECONDS
    if len(sys.argv) > 1:
        try:
            duration = int(sys.argv[1])
        except ValueError:
            print(f"Usage: python serial_logger.py [duration_seconds]")
            sys.exit(1)
    
    # Find Arduino
    port = find_arduino_port()
    if not port:
        print("Error: No Arduino found!")
        sys.exit(1)
    
    # Generate output filename with timestamp
    timestamp = datetime.now().strftime("%Y%m%d_%H%M%S")
    csv_file = os.path.join(OUTPUT_DIR, f"data_{timestamp}.csv")
    
    # Log data
    success = log_serial_data(port, duration, csv_file)
    
    if success:
        # Generate plots
        generate_plots(csv_file, OUTPUT_DIR)
        
        # Also update data.csv for compatibility
        import shutil
        shutil.copy(csv_file, os.path.join(OUTPUT_DIR, "data.csv"))
        print(f"\nAlso copied to: data.csv")
        
        print("\n" + "=" * 50)
        print("  COMPLETE!")
        print("=" * 50)
    else:
        print("No data logged.")

if __name__ == "__main__":
    main()
