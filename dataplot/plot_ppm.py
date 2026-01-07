# import pandas as pd
# import matplotlib.pyplot as plt

# # Read CSV with explicit column names
# df = pd.read_csv(
#     "data.csv",
#     header=None,
#     names=["time_ms", "site", "sensor", "rs_kohm", "rs_ro", "ppm"]
# )

# # Convert time to seconds
# df["time_s"] = df["time_ms"] / 1000

# # Sensor list
# sensors = [
#     ("MQ4_CH4", "MQ-4 Methane (CH₄)"),
#     ("MQ6_LPG", "MQ-6 LPG"),
#     ("MQ8_H2", "MQ-8 Hydrogen (H₂)"),
#     ("MQ135_AIR", "MQ-135 Air Quality")
# ]

# # Create plots
# fig, axes = plt.subplots(2, 2, figsize=(12, 8), sharex=True)
# axes = axes.flatten()

# for ax, (sensor_id, title) in zip(axes, sensors):
#     data = df[df["sensor"] == sensor_id]
#     ax.plot(data["time_s"], data["ppm"], marker='o', linewidth=1)
#     ax.set_title(title)
#     ax.set_ylabel("PPM")
#     ax.grid(alpha=0.3)

# for ax in axes:
#     ax.set_xlabel("Time (s)")

# plt.suptitle("MQ Sensor Gas Concentration vs Time", fontsize=14)
# plt.tight_layout(rect=[0, 0, 1, 0.95])
# plt.show()



# import pandas as pd
# import matplotlib.pyplot as plt

# # 1. Load data
# df = pd.read_csv("data.csv", header=None, 
#                  names=["time_ms", "site", "sensor", "rs_kohm", "rs_ro", "ppm"])

# # 2. Cleanup
# df["time_s"] = df["time_ms"] / 1000
# # Filter out the initial 60s spike for the final report
# df_stable = df[df["time_s"] > 60] 

# # 3. Define Sensor Categories
# mq_sensors = ["MQ4_CH4", "MQ6_LPG", "MQ8_H2", "MQ135_AIR"]
# env_sensors = ["BMP280_TEMP", "BMP280_PRESS"]

# # 4. Create a 3x2 grid to include Temperature and Pressure
# fig, axes = plt.subplots(3, 2, figsize=(14, 10), sharex=True)
# axes = axes.flatten()

# # Plot MQ Sensors
# for i, sensor_id in enumerate(mq_sensors):
#     data = df_stable[df_stable["sensor"] == sensor_id]
#     axes[i].plot(data["time_s"], data["ppm"], color='tab:blue', label='PPM')
#     axes[i].set_title(f"Concentration: {sensor_id}")
#     axes[i].set_ylabel("PPM")
#     axes[i].grid(True, linestyle='--', alpha=0.6)

# # Plot Environmental Data (BMP280)
# # Note: In your current CSV, BMP data might have different units in the 'ppm' column
# temp_data = df_stable[df_stable["sensor"] == "BMP280_TEMP"]
# axes[4].plot(temp_data["time_s"], temp_data["ppm"], color='tab:red')
# axes[4].set_title("Ambient Temperature (Udupi)")
# axes[4].set_ylabel("Celsius (°C)")

# press_data = df_stable[df_stable["sensor"] == "BMP280_PRESS"]
# axes[5].plot(press_data["time_s"], press_data["ppm"], color='tab:green')
# axes[5].set_title("Atmospheric Pressure")
# axes[5].set_ylabel("hPa")

# plt.suptitle("TEAM OBSERACT - Rover Science Mission Data", fontsize=16)
# plt.tight_layout(rect=[0, 0, 1, 0.96])
# plt.show()

# import csv
# import matplotlib.pyplot as plt
# from collections import defaultdict

# CSV_FILE = "data.csv"   # your data file
# TIME_SCALE = 1000.0       # ms → seconds

# time_data = defaultdict(list)
# ppm_data = defaultdict(list)

# # Read CSV
# with open(CSV_FILE, "r") as f:
#     reader = csv.reader(f)
#     for row in reader:
#         if len(row) < 6:
#             continue

#         time_sec = float(row[0]) / TIME_SCALE
#         sensor   = row[2]
#         ppm      = float(row[5])

#         time_data[sensor].append(time_sec)
#         ppm_data[sensor].append(ppm)

# # ====== PLOT EACH SENSOR SEPARATELY ======

# for sensor in time_data:
#     plt.figure(figsize=(10, 4))
#     plt.plot(time_data[sensor], ppm_data[sensor])
#     plt.xlabel("Time (seconds)")
#     plt.ylabel("PPM")
#     plt.title(f"{sensor} : PPM vs Time")
#     plt.grid(True)
#     plt.tight_layout()
#     plt.show()


##---------------------last-----------------------

# import pandas as pd
# import matplotlib.pyplot as plt

# # ---------------- LOAD DATA ----------------
# df = pd.read_csv(
#     "data.csv",
#     header=None,
#     names=["time_ms", "site", "sensor", "rs_kohm", "rs_ro", "ppm"]
# )

# # Convert time to seconds
# df["time_s"] = df["time_ms"] / 1000.0

# # ---------------- SENSOR CONFIG (MATCHES YOUR DATA) ----------------
# sensors = [
#     ("MQ4_CH4",     "MQ-4 Methane (CH₄)"),
#     ("MQ136_H2S",   "MQ-136 Hydrogen Sulfide (H₂S)"),
#     ("MQ8_H2",      "MQ-8 Hydrogen (H₂)"),
#     ("MQ135_AIR",   "MQ-135 Air Quality")
# ]

# # ---------------- PLOT: 4 SENSORS, ONE SCREEN ----------------
# fig, axes = plt.subplots(2, 2, figsize=(15, 9))
# axes = axes.flatten()

# for ax, (sensor_id, title) in zip(axes, sensors):
#     data = df[df["sensor"] == sensor_id]

#     if data.empty:
#         ax.set_title(f"{title}\n(No Data)")
#         ax.axis("off")
#         continue

#     ax.plot(data["time_s"], data["ppm"], linewidth=1.5)
#     ax.set_title(title)
#     ax.set_xlabel("Time (seconds)")
#     ax.set_ylabel("PPM")
#     ax.grid(alpha=0.3)

# plt.suptitle("MQ Sensors – Time vs Gas Concentration (PPM)", fontsize=16)
# plt.tight_layout(rect=[0, 0, 1, 0.95])
# plt.show()


import pandas as pd
import matplotlib.pyplot as plt

# =======================
# LOAD CSV DATA
# =======================
csv_file = "data.csv"
df = pd.read_csv(csv_file)

# Convert time from ms to seconds
df["time_s"] = df["time_ms"] / 1000.0

# =======================
# FILTER FUNCTIONS
# =======================
def plot_sensor(sensor_name, site_id=1):
    data = df[(df["sensor"] == sensor_name) & (df["site"] == site_id)]

    if data.empty:
        print(f"No data for {sensor_name} at site {site_id}")
        return

    plt.figure()
    plt.plot(data["time_s"], data["value"])
    plt.xlabel("Time (s)")
    plt.ylabel(f"{sensor_name} ({data['unit'].iloc[0]})")
    plt.title(f"{sensor_name} vs Time (Site {site_id})")
    plt.grid(True)
    plt.show()

def plot_environment(param, site_id=1):
    data = df[df["site"] == site_id]

    plt.figure()
    plt.plot(data["time_s"], data[param])
    plt.xlabel("Time (s)")
    plt.ylabel(param)
    plt.title(f"{param} vs Time (Site {site_id})")
    plt.grid(True)
    plt.show()

# =======================
# MQ GAS PLOTS (Individual)
# =======================
# plot_sensor("MQ4_CH4", site_id=1)
# plot_sensor("MQ136_H2S", site_id=1)
# plot_sensor("MQ8_H2", site_id=1)
# plot_sensor("MQ135_CO2", site_id=1)

# =======================
# COMBINED MQ SENSOR PLOT (2x2 Grid)
# =======================
def plot_all_sensors_combined(site_id=1):
    sensors = [
        ("MQ4_CH4", "MQ-4 Methane (CH₄)", "tab:blue"),
        ("MQ136_H2S", "MQ-136 H₂S", "tab:orange"),
        ("MQ8_H2", "MQ-8 Hydrogen (H₂)", "tab:green"),
        ("MQ135_CO2", "MQ-135 CO₂", "tab:red")
    ]
    
    fig, axes = plt.subplots(2, 2, figsize=(14, 10))
    axes = axes.flatten()
    
    for ax, (sensor_name, title, color) in zip(axes, sensors):
        data = df[(df["sensor"] == sensor_name) & (df["site"] == site_id)]
        
        if data.empty:
            ax.set_title(f"{title}\n(No Data)")
            ax.axis("off")
            continue
        
        ax.plot(data["time_s"], data["value"], color=color, linewidth=1.5)
        ax.set_title(title, fontsize=12)
        ax.set_xlabel("Time (s)")
        ax.set_ylabel(f"{data['unit'].iloc[0]}")
        ax.grid(True, alpha=0.3)
    
    plt.suptitle("MQ Gas Sensors — Team Obseract Rover", fontsize=16)
    plt.tight_layout(rect=[0, 0, 1, 0.96])
    plt.show()

# =======================
# COMBINED ENVIRONMENT PLOT (1x3 Grid)
# =======================
def plot_all_env_combined(site_id=1):
    env_params = [
        ("BME_TEMP", "Temperature", "°C", "tab:red"),
        ("BME_HUM", "Humidity", "%", "tab:blue"),
        ("BME_PRESS", "Pressure", "hPa", "tab:green")
    ]
    
    fig, axes = plt.subplots(1, 3, figsize=(15, 4))
    
    for ax, (sensor_name, title, unit, color) in zip(axes, env_params):
        data = df[(df["sensor"] == sensor_name) & (df["site"] == site_id)]
        
        if data.empty:
            ax.set_title(f"{title}\n(No Data)")
            continue
        
        ax.plot(data["time_s"], data["value"], color=color, linewidth=1.5)
        ax.set_title(title, fontsize=12)
        ax.set_xlabel("Time (s)")
        ax.set_ylabel(unit)
        ax.grid(True, alpha=0.3)
    
    plt.suptitle("BME280 Environmental Data — Team Obseract Rover", fontsize=14)
    plt.tight_layout(rect=[0, 0, 1, 0.92])
    plt.show()

# =======================
# RUN COMBINED PLOTS
# =======================
plot_all_sensors_combined(site_id=1)
plot_all_env_combined(site_id=1)
