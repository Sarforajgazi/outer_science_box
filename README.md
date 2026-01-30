# Outer Science Box - README

## Overview

Arduino Mega controller for Team Obseract's rover science mission.

**Features:**
- 4x MQ Gas Sensors (CH4, H2S, H2, CO2)
- 1x BME280 Environmental Sensor (temperature, humidity, pressure)
- 1x DHT22 Soil Temperature/Humidity Sensor
- 1x HW-103 Soil Moisture Sensor
- Simulated NPK 7-in-1 Sensor (EC, pH, N, P, K)
- 8-Channel Relay Motor Control (soil collection)
- 2x Servo Motors
- Automated soil drilling sequence

---

## Quick Start (One Command)

```bash
# 1. Upload firmware to Arduino (do this once)
cd /Users/sarforajgazi/outer_science_box
pio run -t upload

# 2. Collect data and generate ALL plots
cd dataplot
source venv/bin/activate
python serial_logger.py
```

This will:
1. Auto-detect Arduino
2. Wait for warmup/calibration
3. Log all sensor data for 7 minutes
4. Generate 4 PNG plot files for all sensors

---

## Hardware Wiring

### Gas & Environmental Sensors
| Component | Pin | Description |
|-----------|-----|-------------|
| BME280 SDA | D20 | I2C Data |
| BME280 SCL | D21 | I2C Clock |
| MQ-4 | A4 | CH4 (Methane) |
| MQ-136 | A6 | H2S (Hydrogen Sulfide) |
| MQ-8 | A0 | H2 (Hydrogen) |
| MQ-135 | A2 | CO2 (Air Quality) |

### Soil Sensors
| Component | Pin | Description |
|-----------|-----|-------------|
| DHT22 DATA | D2 | Soil temperature & humidity |
| HW-103 AO | A9 | Soil moisture (analog) |
| HW-103 DO | D24 | Soil moisture (digital threshold) |

### Motors (Relay Module)
| Relay | Pin | Function |
|-------|-----|----------|
| 1 | 46 | Platform 1 UP |
| 2 | 44 | Platform 1 DOWN |
| 3 | 45 | Platform 2 DOWN |
| 4 | 43 | Platform 2 UP |
| 5 | 41 | Drill (Auger) |
| 6 | 39 | Lift mechanism |
| 7 | 37 | (available) |
| 8 | 35 | (available) |

### Servos
| Servo | Pin |
|-------|-----|
| Small | 19 |
| Big | 42 |

---

## Configuration

Edit `src/main.cpp` (lines 58-69):

```cpp
bool ENABLE_MOTORS = true;              // false = skip motors
unsigned long LOGGING_DURATION_MS = 300000;  // 5 min logging
unsigned long LOG_INTERVAL_MS = 1000;        // 1 sec between readings
unsigned long PLATFORM_MOVE_TIME = 3000;     // 3 sec per move
unsigned long DRILL_TIME = 2000;             // 2 sec drilling
```

### Configuration Options

| Variable | Default | What to Change |
|----------|---------|----------------|
| `ENABLE_MOTORS` | `true` | Set `false` to run sensors only (skip drilling) |
| `LOGGING_DURATION_MS` | `300000` | 60000 = 1 min, 600000 = 10 min |
| `PLATFORM_MOVE_TIME` | `3000` | Increase if platform moves slowly |
| `DRILL_TIME` | `2000` | Increase for deeper drilling |
| `LOG_INTERVAL_MS` | `1000` | 500 = faster logging, 2000 = slower |

### Sensors Only Mode (No Motors)

To run **sensors only** without any motor movement:

1. Open `src/main.cpp`
2. Change line 58:
   ```cpp
   bool ENABLE_MOTORS = false;  // Changed from true to false
   ```
3. Upload and run

---

## Upload Commands

### Find Your Port First

> ⚠️ **Port names are device-specific!** Always detect your actual port before uploading.

```bash
# List available USB ports
ls /dev/cu.usb*

# Or use PlatformIO
pio device list
```

Set your port:
```bash
PORT=/dev/cu.usbserial-XXXX   # Replace XXXX with your actual number
```

### Full Mission (Motors + Sensors)
```bash
pio run -e megaatmega2560 -t upload --upload-port $PORT
pio device monitor -p $PORT -b 9600
```

### Motor Test Only (Manual Control)
```bash
pio run -e relay_test -t upload --upload-port $PORT
pio device monitor -p $PORT -b 9600
```

**Relay Test Commands:**
| Key | Action |
|-----|--------|
| 1-8 | Toggle relay |
| a | All ON |
| o | All OFF |
| s | Sequential test |
| c | Soil collection sequence |
| t | Print status |

---

## Data Logging & Plotting

### One-Command Solution

```bash
cd /Users/sarforajgazi/outer_science_box/dataplot
source venv/bin/activate
python serial_logger.py         # Default: 420 seconds (7 min)
python serial_logger.py 300     # Custom: 300 seconds (5 min)
```

### Output Files Generated

```
dataplot/
├── data_YYYYMMDD_HHMMSS.csv       # Timestamped raw data
├── data.csv                        # Copy for compatibility
├── mq_sensors_*.png                # MQ gas sensors (2x2 grid)
├── environment_*.png               # BME280 data (1x3 grid)
├── soil_sensors_*.png              # DHT22 + HW-103 (1x3 grid)
└── npk_data_*.png                  # NPK analysis (2x3 grid)
```

### All Sensors Logged

| Category | Sensors | Data Logged |
|----------|---------|-------------|
| **Gas** | MQ-4, MQ-136, MQ-8, MQ-135 | CH4, H2S, H2, CO2 (ppm) |
| **Environment** | BME280 | Temperature, Humidity, Pressure |
| **Soil (Real)** | DHT22 | Soil Temperature, Soil Humidity |
| **Soil (Real)** | HW-103 | Soil Moisture (%) |
| **Soil (Simulated)** | NPK 7-in-1 | EC, pH, N, P, K |

### CSV Format
```
time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa
123456,1,MQ4_CH4,18.3,ppm,28.5,65.2,1012.3
123456,1,SOIL_TEMP,26.2,C,28.5,65.2,1012.3
123456,1,NITROGEN,42,mg/kg,28.5,65.2,1012.3
```

---

## Soil Collection Sequence

```
1. Platform 1 DOWN  (3 sec)
2. Drill ON
3. Platform 2 DOWN  (3 sec)
4. Wait             (2 sec)
5. Platform 2 UP    (3 sec)
6. Drill OFF
7. Platform 1 UP    (3 sec)

Total: ~14 seconds
```

---

## File Structure

```
outer_science_box/
├── src/
│   ├── main.cpp              # Main controller (motors + all sensors)
│   ├── MQManager.cpp         # MQ sensor logic
│   └── MQSensor.cpp          # Individual MQ sensor
├── include/
│   ├── RelayController.hpp   # Motor control + sequence
│   ├── MQManager.hpp         # MQ manager header
│   ├── MQSensor.hpp          # MQ sensor header
│   └── NPKSensor.hpp         # NPK sensor (if using real sensor)
├── test/
│   ├── relay_test.cpp        # Manual relay testing
│   └── npk_sensor_test.cpp   # NPK sensor testing
├── dataplot/
│   ├── serial_logger.py      # Data capture + auto-plotting (ALL sensors)
│   ├── plot_ppm.py           # Interactive graph generation
│   ├── serial_to_csv.py      # CSV conversion utility
│   └── venv/                 # Python virtual environment
├── platformio.ini            # Build configuration
└── README.md                 # This file
```

---

## Mission Timeline

| Phase | Duration | Description |
|-------|----------|-------------|
| Startup | ~2 sec | Initialize all sensors, servos, relays |
| MQ Warmup | 120 sec | Heat MQ sensor elements |
| Calibration | ~5 sec | Calibrate MQ sensors in clean air |
| Soil Collection | ~14 sec | Run drill sequence (if motors enabled) |
| Data Logging | 300 sec | Log all sensors every 1 second |
| **Total** | ~7 min | Complete mission |

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Arduino not detected | Check USB cable, try different port |
| Relay module not working | Power relay module separately (5V 2A recommended) |
| Motors skip | Set `ENABLE_MOTORS = false` to test sensors first |
| BME280 not found | Check I2C wiring (SDA→D20, SCL→D21) |
| DHT22 read errors | Check wiring (DATA→D2), add 10kΩ pull-up if needed |
| HW-103 wrong values | Calibrate `MOISTURE_DRY_VALUE` and `MOISTURE_WET_VALUE` |
| No serial output | Check baud rate is 9600 in monitor |
| Plots not generating | Ensure `pandas` and `matplotlib` are installed in venv |

---

## Dependencies

### Arduino Libraries (via PlatformIO)
- `Adafruit BME280 Library`
- `DHT sensor library`
- `Servo`

### Python (in `dataplot/venv`)
```bash
pip install pyserial pandas matplotlib
```

---

## Author

**Team Obseract** - Rover Science Mission
