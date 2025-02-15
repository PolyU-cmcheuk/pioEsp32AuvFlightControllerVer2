#ifndef MOTIONSENSOR_H
#define MOTIONSENSOR_H

// need to include arduino library for function: Delay(),
#include <Arduino.h>

#include <Wire.h>
// motion sensor related libs
#include <Adafruit_MPU6050.h>
// #include <Adafruit_Sensor.h>

// AHRS lib
#include <Adafruit_AHRS.h>

class MotionSensor
{
public:
  MotionSensor(void) : ahrs(ahrsGain) // constructor
  {
  }
  ~MotionSensor(void) {}; // destructor

  // function prototypes
  bool init(void);
  void update(void);
  void calibrateGyro(int numSamples);
  // getta functions
  float getTemp(void);
  float getRobotGyroX(void);
  float getRobotGyroY(void);
  float getRobotGyroZ(void);
  float getRobotAccX(void);
  float getRobotAccY(void);
  float getRobotAccZ(void);
  float getRobotRoll(void);
  float getRobotPitch(void);
  float getRobotYaw(void);

private:
  // mpu related parameters below
  // which is converted to radian unit for quaternion computation
  const float rad2deg = 1 / 3.14159 * 180.0;
  Adafruit_MPU6050 mpu; // mpu6050
  bool mpuInitialized = false;
  sensors_event_t a, g, temp; // mpu raw measurements

  // AHRS related parameters below
  const int framerate = 50;
  const float ahrsGain = 0.3; // need to manually tune this value according to the framerate to achieve fastest convergency
  Adafruit_Madgwick ahrs;     // gain of Madgwick filter, default=0.1,

  // variables to
  float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

  // update IMU, map IMU frame to Robot frame
  // IMU-X --> Robot-Forward
  // IMU-Y --> Robot-Leftward (but Rightward is positive)
  // IMU-Z --> Robot-Downward positive, z=0 at surface
  // note: ahrs lib requires gyro input in deg/s, not rad/s
  float robotGyroX, robotGyroY, robotGyroZ;
  float robotAccX, robotAccY, robotAccZ;
  // robot roll pitch yaw angles, note that yaw angle may drift a lot
  float robotRoll;
  float robotPitch;
  float robotYaw;
};

#endif // MOTIONSENSOR_H