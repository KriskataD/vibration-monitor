#include <Arduino.h>
#include <Wire.h>
#include <MPU6050.h>
#include <Ticker.h>

#define SDA_PIN 4
#define SCL_PIN 5

#define ACCEL_SCALE 16384.0
#define GYRO_SCALE  131.0

#define SAMPLE_RATE_HZ    400
#define SAMPLE_INTERVAL_S (1.0f / SAMPLE_RATE_HZ)
#define WINDOW_MS         500

struct WindowRMS {
  float sumSq = 0;
  int   count = 0;
  void addSample(float x) { sumSq += x * x; count++; }
  float finish() {
    float rms = (count > 0) ? sqrt(sumSq / count) : 0;
    sumSq = 0; count = 0;
    return rms;
  }
};

MPU6050 mpu;
Ticker  sampler;

volatile bool sampleReady = false;

WindowRMS accelWindow;
WindowRMS gyroWindow;
unsigned long windowStart = 0;

void IRAM_ATTR onTick() {
  sampleReady = true;
}

void setup() {
  Serial.begin(921600);
  Wire.begin(SDA_PIN, SCL_PIN);
  Wire.setClock(400000);

  mpu.initialize();

  if (!mpu.testConnection()) {
    Serial.println("# MPU6050 connection failed");
    while (true);
  }

  Serial.println("# Calibrating — keep sensor still...");
  mpu.CalibrateAccel(6);
  mpu.CalibrateGyro(6);
  Serial.println("# Calibration done.");

  Serial.println("t_ms,ax_g,ay_g,az_g,gx_dps,gy_dps,gz_dps");

  windowStart = millis();
  sampler.attach(SAMPLE_INTERVAL_S, onTick);
}

void loop() {
  if (!sampleReady) return;
  sampleReady = false;

  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  float ax_g   = ax / ACCEL_SCALE;
  float ay_g   = ay / ACCEL_SCALE;
  float az_g   = az / ACCEL_SCALE;
  float gx_dps = gx / GYRO_SCALE;
  float gy_dps = gy / GYRO_SCALE;
  float gz_dps = gz / GYRO_SCALE;

  float accelMag = sqrt(ax_g*ax_g + ay_g*ay_g + az_g*az_g);
  float gyroMag  = sqrt(gx_dps*gx_dps + gy_dps*gy_dps + gz_dps*gz_dps);

  accelWindow.addSample(accelMag);
  gyroWindow.addSample(gyroMag);

  unsigned long now = millis();
  Serial.print(now);        Serial.print(",");
  Serial.print(ax_g, 4);   Serial.print(",");
  Serial.print(ay_g, 4);   Serial.print(",");
  Serial.print(az_g, 4);   Serial.print(",");
  Serial.print(gx_dps, 4); Serial.print(",");
  Serial.print(gy_dps, 4); Serial.print(",");
  Serial.println(gz_dps, 4);

  if (now - windowStart >= WINDOW_MS) {
    float accelRMS = accelWindow.finish();
    float gyroRMS  = gyroWindow.finish();
    float ratioRMS = (gyroRMS > 0) ? accelRMS / gyroRMS : 0;

    Serial.print("# RMS | accel=");  Serial.print(accelRMS, 4);
    Serial.print(" g | gyro=");      Serial.print(gyroRMS, 4);
    Serial.print(" dps | ratio=");   Serial.println(ratioRMS, 4);

    windowStart = now;
  }
}
