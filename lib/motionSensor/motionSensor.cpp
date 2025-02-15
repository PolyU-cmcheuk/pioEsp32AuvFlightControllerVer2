#include <motionSensor.h>

// constructor and destructor initialized inside the class declaration.
// because we need to use initialize list for some objects of other class

bool MotionSensor::init(void)
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
  return mpuInitialized;
}

void MotionSensor::update(void)
{
  // read raw measurement from IMU
  mpu.getEvent(&a, &g, &temp);
  // update IMU, map IMU frame to Robot frame
  // IMU-X --> Robot-Forward
  // IMU-Y --> Robot-Leftward (but Rightward is positive)
  // IMU-Z --> Robot-Downward positive, z=0 at surface
  // note: ahrs lib requires gyro input in deg/s, not rad/s
  robotGyroX = g.gyro.x * rad2deg;
  robotGyroY = g.gyro.y * rad2deg;
  robotGyroZ = g.gyro.z * rad2deg;
  robotAccX = a.acceleration.x;
  robotAccY = a.acceleration.y;
  robotAccZ = a.acceleration.z;

  // compute AHRS, also need to align the axis to robot frame coordinate system as aeroplane
  ahrs.updateIMU(robotGyroX, robotGyroY, robotGyroZ, robotAccX, robotAccY, robotAccZ);
  robotRoll = ahrs.getRoll();
  robotPitch = -ahrs.getPitch();
  robotYaw = ahrs.getYaw();
}
// getta functions
float MotionSensor::getTemp(void) { return temp.temperature; }
float MotionSensor::getRobotGyroX(void) { return robotGyroX; }
float MotionSensor::getRobotGyroY(void) { return robotGyroY; }
float MotionSensor::getRobotGyroZ(void) { return robotGyroZ; }
float MotionSensor::getRobotAccX(void) { return robotAccX; }
float MotionSensor::getRobotAccY(void) { return robotAccY; }
float MotionSensor::getRobotAccZ(void) { return robotAccZ; }
float MotionSensor::getRobotRoll(void) { return robotRoll; }
float MotionSensor::getRobotPitch(void) { return robotPitch; }
float MotionSensor::getRobotYaw(void) { return robotYaw; }