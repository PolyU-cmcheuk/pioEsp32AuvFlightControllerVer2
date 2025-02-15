#ifndef THRUSTERCONTROL_H
#define THRUSTERCONTROL_H

// need to include arduino library for function: analogRead()
#include <Arduino.h>
// thruster, servo control related libs
#include <ESP32Servo.h>

class ThrusterControl
{
public:
  enum SpinDirection // spinning direction (thruster / esc only)
  {
    CW, // clockwise
    CCW // counter-clockwise
  };

public:
  ThrusterControl(int pwmPin);                                                                                        // must provide pwm output pins to constructor
  ThrusterControl(int pwmPin, int pwmFreq, int pwmMinUs, int pwmMaxUs, int pwmMiddleUs, SpinDirection spinDirection); // must provide pwm output pins to constructor
  ~ThrusterControl() {};

  // function for updating all parameters after configuring any one of them
  bool updateParameters(void);
  // functions for configuring parameters
  bool setPwmFreq(int pwmFreq);
  bool setPwmMinUs(int pwmMinUs);
  bool setPwmMaxUs(int pwmMaxUs);
  bool setPwmParameters(int pwmFreq, int pwmMinUs, int pwmMaxUs);
  bool setPwmValue(int pwmValueUs);
  bool setPwmValueNormalized(float pwmNormalized);
  // getta functions
  int getPwmValue();

  // other functions
  bool isValidSpinDirection(SpinDirection spinDirection); // function for check valid mode
  // quick reusable functions
  float mapFloat(float input, float inMin, float inMax, float outMin, float outMax);

private:
  // hardware related parameters
  int _pwmPin;          // pwm (servo standard) output pin
  int _pwmValueUs;      // pwm value, ranged within [pwmMin, pwmMax]
  int _pwmFreq = 100;   // set to 50Hz for controlling cheap servo motors, but it can be higher as 400Hz for most ESCs
  int _pwmMinUs = 1100; // unit = us
  int _pwmMaxUs = 1900; // unit = us
  int _pwmRange = _pwmMaxUs - _pwmMinUs;
  int _pwmMiddle = 1480; // need to adjust for it to make sure it started up robustly
  // Servo class from ESP32Servo.h, create pwm object to use the function inside
  Servo pwm;
  SpinDirection _spinDirection = CW; // default:CW from purchase
};

#endif // THRUSTERCONTROL_H