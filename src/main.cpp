#include <Arduino.h>

#include "Timer.h"
Timer timer; // default resolution = MILLIS, use MILLIS for tickers over 70 min
// Timer timer(MICROS);

// custom kill switch checking function
#include <killSwitch.h>

// custom test reading from potentionmeter for dirty test
#include <testReadPotentionmeter.h>

// custom display lib
#include <oled0960A.h>

// motion sensor related libs
#include <motionSensor.h>
MotionSensor mpu6050;

// water pressure sensor (bar30)
#include <waterPressureSensor.h>
WaterPressureSensor pressureSensor;
float depthTarget = -0.1; // unit = m below water surface, if(-ve): disabled depth hold mode

// thruster, servo control related libs
#include <thrusterControl.h>
// thrusters (servo) related
// thruster pins and some constants
const int thrusterLFpin = 32;  // GPIO#
const int thrusterRFpin = 33;  // GPIO#
const int thrusterLMUpin = 25; // GPIO#
const int thrusterRMUpin = 26; // GPIO#
const int thrusterLBpin = 27;  // GPIO#
const int thrusterRBpin = 14;  // GPIO#
const int thrusterLMDpin = 12; // GPIO#
const int thrusterRMDpin = 13; // GPIO#
// pwm value related config
const int thrusterPwmFreq = 200;
const int thrusterPwmMinUs = 1100;
const int thrusterPwmMaxUs = 1900;
// need to adjust for it to make sure it started robustly
const int thrusterPwmMiddle = 1450;
ThrusterControl thrusterLF(thrusterLFpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CW);    // Left Front,
ThrusterControl thrusterRF(thrusterRFpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CCW);   // Right Front,
ThrusterControl thrusterLMU(thrusterLMUpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CW);  // Left Middle Up
ThrusterControl thrusterRMU(thrusterRMUpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CCW); // Right Middle Up
ThrusterControl thrusterLB(thrusterLBpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CW);    // Left Back
ThrusterControl thrusterRB(thrusterRBpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CCW);   // Right Back
ThrusterControl thrusterLMD(thrusterLMDpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CCW); // Left Middle Down, optional: can be removed to return to normal fifish mode
ThrusterControl thrusterRMD(thrusterRMDpin, thrusterPwmFreq, thrusterPwmMinUs, thrusterPwmMaxUs, thrusterPwmMiddle, ThrusterControl::CW);  // Right Middle Down, optional: can be removed to return to normal fifish mode

// dirty code here first
enum ThrusterArmMode
{
  DISARMED, // shutdown all thrusters
  ARMING,   // running the ESC initialization sequences, it may sweep all the pwm channels
  ARMED     // armed mode, enable auto / manual control
};
ThrusterArmMode thrusterArmMode = DISARMED;

// dirty test functions for all pwm
void setAllThrusterPwm(int pwmValue)
{
  thrusterLF.setPwmValue(pwmValue);
  thrusterRF.setPwmValue(pwmValue);
  thrusterLMU.setPwmValue(pwmValue);
  thrusterRMU.setPwmValue(pwmValue);
  thrusterLB.setPwmValue(pwmValue);
  thrusterRB.setPwmValue(pwmValue);
  thrusterLMD.setPwmValue(pwmValue);
  thrusterRMD.setPwmValue(pwmValue);
}
void setAllThrusterPwmNormalized(float pwmValueNormalized)
{
  thrusterLF.setPwmValueNormalized(pwmValueNormalized);
  thrusterRF.setPwmValueNormalized(pwmValueNormalized);
  thrusterLMU.setPwmValueNormalized(pwmValueNormalized);
  thrusterRMU.setPwmValueNormalized(pwmValueNormalized);
  thrusterLB.setPwmValueNormalized(pwmValueNormalized);
  thrusterRB.setPwmValueNormalized(pwmValueNormalized);
  thrusterLMD.setPwmValueNormalized(pwmValueNormalized);
  thrusterRMD.setPwmValueNormalized(pwmValueNormalized);
}
void armAllThrusters(void)
{
  int delayMs = 100, initialDelayMs = 500;
  float sweepMin = -0.2, sweepMax = 0.2, sweepStep = 0.02;

  // delay for Thrusters initialization
  setAllThrusterPwmNormalized(0.0);
  delay(initialDelayMs);

  // start sweeping
  for (float i = sweepMin; i < sweepMax; i += sweepStep)
  {
    setAllThrusterPwmNormalized(i);
    delay(delayMs);
  }
  for (float i = sweepMax; i > sweepMin; i -= sweepStep)
  {
    setAllThrusterPwmNormalized(i);
    delay(delayMs);
  }

  // finally wait at the middle value.
  setAllThrusterPwmNormalized(0.0);
  delay(initialDelayMs);
}
/**
 * @brief speed control by keyboard / ASCII char input
 *
 */
float forwardSpeed = 0.0;
float rightwardSpeed = 0.0;
float downwardSpeed = 0.0;
float yawSpeed = 0.0;
float pitchSpeed = 0.0;
const int keyLiveCounter = 10; // e.g. 10 frame
unsigned int forwardSpeedKeyLiveCounter = 0;
unsigned int rightwardSpeedKeyLiveCounter = 0;
unsigned int downwardSpeedKeyLiveCounter = 0;
unsigned int yawSpeedKeyLiveCounter = 0;
unsigned int pitchSpeedKeyLiveCounter = 0;
// for setting target yaw angle
float targetYaw = 0.0;

/**
 * @brief
 *
 * @param forwardCtrl +ve
 * @param rightwardCtrl +ve
 * @param downwardCtrl +ve
 * @param rollCtrl not used in narrow fifish frame
 * @param pitchCtrl +ve for nose up
 * @param yawCtrl +ve for yaw right about downward Z-axis
 */
void robotSpeedCtrl(float forwardCtrl, float rightwardCtrl, float downwardCtrl, float rollCtrl, float pitchCtrl, float yawCtrl)
{
  // note: roll is disabled in Alpha's frame due to slim and narrow config
  // float rollCtrlGain = 0.0;
  float pitchCtrlGain = 0.9;
  float yawCtrlGain = 0.9;

  // temp values for different thrusters
  // note: gains are set only temporily, not related to geometry
  float LFvalue = constrain((-downwardCtrl + rightwardCtrl + pitchCtrl * pitchCtrlGain), -1.0, 1.0);
  float RFvalue = constrain((-downwardCtrl - rightwardCtrl + pitchCtrl * pitchCtrlGain), -1.0, 1.0);
  float LBvalue = constrain((-downwardCtrl - rightwardCtrl - pitchCtrl * pitchCtrlGain), -1.0, 1.0);
  float RBvalue = constrain((-downwardCtrl + rightwardCtrl - pitchCtrl * pitchCtrlGain), -1.0, 1.0);

  float LMUvalue = constrain((forwardCtrl + yawCtrl * yawCtrlGain), -1.0, 1.0);
  float RMUvalue = constrain((forwardCtrl - yawCtrl * yawCtrlGain), -1.0, 1.0);
  float LMDvalue = constrain((forwardCtrl + yawCtrl * yawCtrlGain), -1.0, 1.0);
  float RMDvalue = constrain((forwardCtrl - yawCtrl * yawCtrlGain), -1.0, 1.0);

  // upward, picth, roll(disabled),
  thrusterLF.setPwmValueNormalized(LFvalue);
  thrusterRF.setPwmValueNormalized(RFvalue);
  thrusterLB.setPwmValueNormalized(LBvalue);
  thrusterRB.setPwmValueNormalized(RBvalue);

  // forward and yaw controls
  thrusterLMU.setPwmValueNormalized(LMUvalue);
  thrusterRMU.setPwmValueNormalized(RMUvalue);
  thrusterLMD.setPwmValueNormalized(LMDvalue);
  thrusterRMD.setPwmValueNormalized(RMDvalue);
}

/**
 * @brief simple Serial event emulating keyboard control,
 * w/a/s/d  := forward/rightward motion
 * q/e      := yaw left/right
 * r/f      := up/down vertically
 * t/g      := pitch up/down
 */
void WASDHandler()
{
  while (Serial.available())
  {
    char incomingByte = Serial.read();
    float speedSlow = 0.4;
    float speedFast = 0.6;
    float targetYawIncrement = 5.0;
    switch (incomingByte)
    {
    case 'w':
      forwardSpeed = speedSlow;
      forwardSpeedKeyLiveCounter = keyLiveCounter;
      break;
    case 's':
      forwardSpeed = -speedSlow;
      forwardSpeedKeyLiveCounter = keyLiveCounter;
      break;
    case 'a':
      rightwardSpeed = -speedSlow;
      rightwardSpeedKeyLiveCounter = keyLiveCounter;
      break;
    case 'd':
      rightwardSpeed = speedSlow;
      rightwardSpeedKeyLiveCounter = keyLiveCounter;
      break;
    case 'r':
      // // manual speed control mode
      // downwardSpeed = -speedSlow;
      // downwardSpeedKeyLiveCounter = keyLiveCounter;
      // depth-hold mode, incrementally update target depth value
      depthTarget -= 0.1; // float up, z-axis pointing downward
      break;
    case 'f':
      // // manual speed control mode
      // downwardSpeed = speedSlow;
      // downwardSpeedKeyLiveCounter = keyLiveCounter;
      // depth-hold mode, incrementally update target depth value
      depthTarget += 0.1; // float up, z-axis pointing downward
      break;
    case 'q':
      // // speed control mode
      // yawSpeed = speedSlow;
      // yawSpeedKeyLiveCounter = keyLiveCounter;
      // // angular position control mode
      targetYaw -= targetYawIncrement;
      if (targetYaw > 360)
        targetYaw -= 360.0;
      else if (targetYaw < 0.0)
        targetYaw += 360.0;
      break;
    case 'e':
      // // speed control mode
      // yawSpeed = speedSlow;
      // yawSpeedKeyLiveCounter = keyLiveCounter;
      // // angular position control mode
      targetYaw += targetYawIncrement;
      if (targetYaw > 360)
        targetYaw -= 360.0;
      else if (targetYaw < 0.0)
        targetYaw += 360.0;
      break;
    case 't':
      pitchSpeed = speedSlow;
      pitchSpeedKeyLiveCounter = keyLiveCounter;
      break;
    case 'g':
      pitchSpeed = -speedSlow;
      pitchSpeedKeyLiveCounter = keyLiveCounter;
      break;

    case 'z': // turn back 180 deg, specifically for SAUVC qualification task
      targetYaw += 180.0;
      if (targetYaw > 360)
        targetYaw -= 360.0;
      else if (targetYaw < 0.0)
        targetYaw += 360.0;
      break;
    default: // nothing for default case
      break;
    }
  }
}
/**
 * @brief update counter, when key not pressed for a period, automatically set speed to 0
 *
 */
void WASDKeyCounterUpdate()
{
  (forwardSpeedKeyLiveCounter > 0) ? forwardSpeedKeyLiveCounter-- : forwardSpeed = 0.0;
  (rightwardSpeedKeyLiveCounter > 0) ? rightwardSpeedKeyLiveCounter-- : rightwardSpeed = 0.0;
  (downwardSpeedKeyLiveCounter > 0) ? downwardSpeedKeyLiveCounter-- : downwardSpeed = 0.0;
  (yawSpeedKeyLiveCounter > 0) ? yawSpeedKeyLiveCounter-- : yawSpeed = 0.0;
  (pitchSpeedKeyLiveCounter > 0) ? pitchSpeedKeyLiveCounter-- : pitchSpeed = 0.0;
}
void serialEvent()
{
  WASDHandler();
}

void setup()
{
  // init serial port,
  // for setting 921600 baudrate: -> open setting (UI) -> search "Serial monitor" -> custom baudrate -> "add item" -> type 921600 or higher rate
  Serial.begin(921600);

  // init oled display360.0 - ahrs.getYaw();
  displayInit();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("AUV Controller");
  display.display();

  // init motion sensors
  mpu6050.init();
  // calibrate gyro drift by simple averaging
  mpu6050.calibrateGyro(30);

  // init water pressure sensor
  pressureSensor.init();
  pressureSensor.read(); // need to read once for getting valid data before next non-blocking read

  // init the first time for later computation for loop time usage
  timer.start();
}

void loop()
{

  // setup display
  display.clearDisplay();
  display.setCursor(0, 0);

  // update sensor measurements
  mpu6050.update();

  // // update pressure sensor (disabled for faster framerate)
  // pressureSensor.read();
  // update pressure sensor by non-blocking read function
  pressureSensor.nonBlockRead();

  // w/a/s/d/ control handling here, controlling from either program or keyboard
  WASDKeyCounterUpdate();

  // disarm all thrusters
  if (isKillSwitchActivated())
  {
    thrusterArmMode = DISARMED;
    display.println("Disarmed");
    // set all thrusters to 0.0, but it actually power off...
    setAllThrusterPwmNormalized(0.0);
  }
  else // arm all thrusters
  {
    // just after re-enabling the kill-switch, previous mode should still be DISARMED
    if (thrusterArmMode == DISARMED)
    {
      // added code for gyro calibration
      mpu6050.calibrateGyro(30);

      // execute arming sequence
      thrusterArmMode = ARMING;
      // show status on display
      display.println("Arming");
      display.display();
      // actual arming all thrusters,
      // note: it may stuck inside for looping and delays here
      armAllThrusters();
      // finished arming
      thrusterArmMode = ARMED;
    }
    else if (thrusterArmMode == ARMED)
    {
      display.println("Armed");
    }
  }

  // simple PID control here,
  // angle: roll control is disabled in our narrow frame
  // angle: pitch control is by PID with pitch angle from motion-sensor
  float pitchPid_pGain = 0.05;
  float pitchPidCtrl = (0.0 - mpu6050.getRobotPitch()) * pitchPid_pGain;
  // angle: yaw control by PID with yaw angle, with target angle input
  float yawPid_pGain = 0.1;
  float yawError = targetYaw - mpu6050.getRobotYaw();
  // handling for angle transition point
  if (yawError > 180.0)
    yawError -= 360;
  if (yawError < -180.0)
    yawError += 360.0;
  // compute yawPid
  float yawPidCtrl = yawError * yawPid_pGain;
  // forwardSpeed, rightwardSpeed is controlled directly as open-loop, no feedback here
  // downwardSpeed may be controlled by open-loop (without sensor) or depth-pid control (with sensor)
  // depth-hold with pressure sensor and thruster control
  float depthPid_pGain = 1.0;
  float depthError = depthTarget - pressureSensor.getDepth();
  float depthHoldPidCtrl = depthError * depthPid_pGain;

  // enable control only in ARMED mode
  if (thrusterArmMode == ARMED)
  {
    if (depthTarget >= 0.0)
    {
      // mode = depth hold + pitch stablized
      robotSpeedCtrl(forwardSpeed, rightwardSpeed, depthHoldPidCtrl, 0, pitchPidCtrl, yawPidCtrl);
    }
    else
    {
      // mode = pitch stablized only
      robotSpeedCtrl(forwardSpeed, rightwardSpeed, downwardSpeed, 0, pitchPidCtrl, yawPidCtrl);
    }

    // display pwm value for debugging
    display.setCursor(0, 16);
    display.println(thrusterLF.getPwmValue());
  }

  // // test control by pot
  // int potPin = 39;
  // float potValueNormalized = testReadPotentionmeterNomalized(potPin, -1.0, 1.0);

  // Serial.print(potValueNormalized);

  // read time and compute usage
  timer.stop();
  uint time1 = timer.read();
  timer.start();

  Serial.print(time1);
  Serial.println();

  // print the time on OLEDd display
  display.setCursor(60, 0);
  display.print(time1);
  // print the motion measurements on display
  display.setCursor(0, 8);
  display.print(mpu6050.getRobotPitch());
  display.setCursor(40, 8);
  display.print(mpu6050.getRobotYaw());
  display.setCursor(80, 8);
  display.print(mpu6050.getTemp());
  // print water pressure and display
  display.setCursor(0, 16);
  display.print(pressureSensor.getDepth());
  display.setCursor(40, 16);
  display.print(pressureSensor.getTemperature());
  display.setCursor(80, 16);
  display.print(depthTarget);

  // OLED display consumes about 13ms below
  display.display();

  // delay(10);
}
