import pandas as pd
import matplotlib.pyplot as plt
import os

# =======================
# LOAD CSV DATA
# =======================
# Use the script's directory to find data.csv (works from any working directory)
script_dir = os.path.dirname(os.path.abspath(__file__))
csv_file = os.path.join(script_dir, "data.csv")
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
