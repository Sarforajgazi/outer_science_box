# Outer Science Box - README

## Overview

Arduino Mega controller for Team Obseract's rover science mission.

**Features:**
- 4x MQ Gas Sensors (CH4, H2S, H2, CO2)
- BME280 Environmental Sensor (temp, humidity, pressure)
- 8-Channel Relay Motor Control (soil collection)
- Automated soil drilling sequence

---

## Hardware Wiring

### Sensors
| Component | Pin | Description |
|-----------|-----|-------------|
| BME280 SDA | D20 | I2C Data |
| BME280 SCL | D21 | I2C Clock |
| MQ-4 | A4 | CH4 (Methane) |
| MQ-136 | A6 | H2S (Hydrogen Sulfide) |
| MQ-8 | A0 | H2 (Hydrogen) |
| MQ-135 | A2 | CO2 (Air Quality) |

### Motors (Relay Module)
| Relay | Pin | Function |
|-------|-----|----------|
| 1 | 46 | Platform 1 UP |
| 2 | 44 | Platform 1 DOWN |
| 3 | 45 | Platform 2 DOWN |
| 4 | 43 | Platform 2 UP |
| 5 | 41 | Drill (Auger) |
| 6 | 39 | Lift mechanism |
| 7 | 37 | (unused) |
| 8 | 35 | (unused) |

### Servos
| Servo | Pin |
|-------|-----|
| Small | 19 |
| Big | 42 |

---

## Configuration

Edit `src/main.cpp` (lines 50-65):

```cpp
bool ENABLE_MOTORS = true;              // false = skip motors
unsigned long LOGGING_DURATION_MS = 300000;  // 5 min logging
unsigned long LOG_INTERVAL_MS = 1000;        // 1 sec between readings
unsigned long PLATFORM_MOVE_TIME = 3000;     // 3 sec per move
unsigned long DRILL_TIME = 2000;             // 2 sec drilling
```

### What Each Variable Does

| Variable | Default | What to Change |
|----------|---------|----------------|
| `ENABLE_MOTORS` | `true` | Set `false` to run sensors only (skip drilling) |
| `LOGGING_DURATION_MS` | `300000` | 60000 = 1 min, 600000 = 10 min |
| `PLATFORM_MOVE_TIME` | `3000` | Increase if platform moves slowly |
| `DRILL_TIME` | `2000` | Increase for deeper drilling |
| `LOG_INTERVAL_MS` | `1000` | 500 = faster logging, 2000 = slower |

### Sensors Only Mode (No Motors)

To run **only MQ sensors and BME280** without any motor movement:

1. Open `src/main.cpp`
2. Change line 52:
   ```cpp
   bool ENABLE_MOTORS = false;  // Changed from true to false
   ```
3. Upload:
   ```bash
   pio run -e megaatmega2560 -t upload --upload-port /dev/cu.usbserial-1140
   ```

This will:
- ✅ Initialize sensors
- ✅ Warmup MQ sensors (120 sec)
- ✅ Calibrate sensors
- ⏭️ Skip soil collection
- ✅ Log sensor data for 5 minutes

---

## Upload Commands

### Find Your Port First

> ⚠️ **Port names are device-specific!** The port (e.g., `/dev/cu.usbserial-1140`) depends on your USB adapter's serial number. Always detect your actual port before uploading.

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

## Data Logging

**Output format (CSV):**
```
time_ms,site,sensor,value,unit,temp_C,hum_%,press_hPa
```

**Capture data (with venv and duration):**
```bash
cd /Users/sarforajgazi/outer_science_box/dataplot
source venv/bin/activate
python serial_logger.py 300    # 300 = log for 5 minutes
```

The logger will:
1. Auto-detect Arduino port
2. Log sensor data for specified duration
3. Save to `data.csv`
4. Auto-generate plots

**Generate graphs manually:**
```bash
python plot_ppm.py
```

---

## File Structure

```
outer_science_box/
├── src/
│   ├── main.cpp           # Main controller (motors + sensors)
│   ├── MQManager.cpp      # MQ sensor logic
│   └── MQSensor.cpp       # Individual sensor
├── include/
│   ├── RelayController.hpp  # Motor control + sequence
│   ├── MQManager.hpp
│   └── MQSensor.hpp
├── test/
│   └── relay_test.cpp     # Manual relay testing
└── dataplot/
    ├── serial_logger.py   # Data capture
    └── plot_ppm.py        # Graph generation
```

---

## Troubleshooting

| Problem | Solution |
|---------|----------|
| Arduino not detected | Check USB cable, try different port |
| Relay disconnects | Power relay module separately |
| Motors skip | Set `ENABLE_MOTORS = false` |
| BME280 not found | Check I2C wiring (SDA/SCL) |
