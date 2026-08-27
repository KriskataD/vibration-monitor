#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>

#define SDA_PIN 4
#define SCL_PIN 5

// Scale factors from MPU6050 datasheet for default ranges (±2g, ±250°/s)
#define ACCEL_SCALE 16384.0
#define GYRO_SCALE  131.0

MPU6050 mpu;

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("# MPU6050 connection failed");
    while (true);
  }

  Serial.println("# Calibrating — keep sensor still...");
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
  Serial.println("# Calibration done.");

  // CSV header — Python script starts recording from here
  Serial.println("ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps");
}

void loop() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  Serial.print(ax / ACCEL_SCALE, 4); Serial.print(",");
  Serial.print(ay / ACCEL_SCALE, 4); Serial.print(",");
  Serial.print(az / ACCEL_SCALE, 4); Serial.print(",");
  Serial.print(gx / GYRO_SCALE,  4); Serial.print(",");
  Serial.print(gy / GYRO_SCALE,  4); Serial.print(",");
  Serial.println(gz / GYRO_SCALE, 4);

  delay(5);
}
