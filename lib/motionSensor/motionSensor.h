#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

// need to include arduino library for function: Delay(),
#include <Arduino.h>

// motion sensor related libs
#include <Adafruit_MPU6050.h>
// #include <Adafruit_Sensor.h>
#include <Wire.h>
// AHRS lib
#include <Adafruit_AHRS.h>

class MotionSensor
{
public:
  MotionSensor(void) : ahrs(ahrsGain)
  {
    mpuInitialized = mpu.begin(); // init MPU6050 sensor
    if (mpuInitialized)
    {
      mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
      mpu.setGyroRange(MPU6050_RANGE_500_DEG);
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
      // set update frequency for AHRS filter
      ahrs.begin(framerate);
    }
  }
  ~MotionSensor(void) {};

  // function prototypes

private:
  // mpu related parameters below
  // which is converted to radian unit for quaternion computation
  const float rad2deg = 1 / 3.14159 * 180.0;
  Adafruit_MPU6050 mpu; // mpu6050
  bool mpuInitialized = false;
  sensors_event_t a, g, temp; // mpu raw measurements

  // AHRS related parameters below
  const int framerate = 100;
  const float ahrsGain = 0.2; // need to manually tune this value according to the framerate to achieve fastest convergency
  Adafruit_Madgwick ahrs;     // gain of Madgwick filter, default=0.1,
};

#endif // MOTIONSENSOR_H