import serial
import os

PORT = "COM3"
BAUD = 115200
CSV_FILE = "data.csv"

if os.path.exists(CSV_FILE):
    os.remove(CSV_FILE)
    print(f"Deleted old {CSV_FILE}")

print(f"Connecting to {PORT}...")
ser = serial.Serial(PORT, BAUD, timeout=1)
print("Connected. Waiting for calibration to finish...")

with open(CSV_FILE, "w", newline="") as f:
    recording = False
    try:
        while True:
            line = ser.readline().decode("utf-8", errors="ignore").strip()
            if not line:
                continue

            if line.startswith("#"):
                print(line[2:])  # print status messages without the #
                continue

            if line.startswith("ax_g"):
                recording = True
                f.write(line + "\n")
                f.flush()
                print(f"Recording to {CSV_FILE} — press Ctrl+C to stop.")
                continue

            if recording:
                f.write(line + "\n")
                f.flush()

    except KeyboardInterrupt:
        print(f"\nStopped. Data saved to {CSV_FILE}")
    finally:
        ser.close()
