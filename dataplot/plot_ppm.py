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



import pandas as pd
import matplotlib.pyplot as plt

# 1. Load data
df = pd.read_csv("data.csv", header=None, 
                 names=["time_ms", "site", "sensor", "rs_kohm", "rs_ro", "ppm"])

# 2. Cleanup
df["time_s"] = df["time_ms"] / 1000
# Filter out the initial 60s spike for the final report
df_stable = df[df["time_s"] > 60] 

# 3. Define Sensor Categories
mq_sensors = ["MQ4_CH4", "MQ6_LPG", "MQ8_H2", "MQ135_AIR"]
env_sensors = ["BMP280_TEMP", "BMP280_PRESS"]

# 4. Create a 3x2 grid to include Temperature and Pressure
fig, axes = plt.subplots(3, 2, figsize=(14, 10), sharex=True)
axes = axes.flatten()

# Plot MQ Sensors
for i, sensor_id in enumerate(mq_sensors):
    data = df_stable[df_stable["sensor"] == sensor_id]
    axes[i].plot(data["time_s"], data["ppm"], color='tab:blue', label='PPM')
    axes[i].set_title(f"Concentration: {sensor_id}")
    axes[i].set_ylabel("PPM")
    axes[i].grid(True, linestyle='--', alpha=0.6)

# Plot Environmental Data (BMP280)
# Note: In your current CSV, BMP data might have different units in the 'ppm' column
temp_data = df_stable[df_stable["sensor"] == "BMP280_TEMP"]
axes[4].plot(temp_data["time_s"], temp_data["ppm"], color='tab:red')
axes[4].set_title("Ambient Temperature (Udupi)")
axes[4].set_ylabel("Celsius (°C)")

press_data = df_stable[df_stable["sensor"] == "BMP280_PRESS"]
axes[5].plot(press_data["time_s"], press_data["ppm"], color='tab:green')
axes[5].set_title("Atmospheric Pressure")
axes[5].set_ylabel("hPa")

plt.suptitle("TEAM OBSERACT - Rover Science Mission Data", fontsize=16)
plt.tight_layout(rect=[0, 0, 1, 0.96])
plt.show()