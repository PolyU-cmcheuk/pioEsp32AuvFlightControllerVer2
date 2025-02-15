#ifndef THRUSTERFRAMECONTROL_H
#define THRUSTERFRAMECONTROL_H

// need to include arduino library for function: analogRead()
#include <Arduino.h>
// need to include my thruster control library
#include <thrusterControl.h>

class ThrusterFrameControl
{
public:
  ThrusterFrameControl(int pwmPin);                                                                                        // must provide pwm output pins to constructor
  ThrusterFrameControl(int pwmPin, int pwmFreq, int pwmMinUs, int pwmMaxUs, int pwmMiddleUs, SpinDirection spinDirection); // must provide pwm output pins to constructor
  ~ThrusterFrameControl() {};

private:
  // hardware related parameters
  // thruster pins and some constants
  const int ThLFpin = 32;  // GPIO#
  const int ThRFpin = 33;  // GPIO#
  const int ThLMUpin = 25; // GPIO#
  const int ThRMUpin = 26; // GPIO#
  const int ThLBpin = 27;  // GPIO#
  const int ThRBpin = 14;  // GPIO#
  const int ThLMDpin = 12; // GPIO#
  const int ThRMDpin = 13; // GPIO#
  // pwm value related config
  const int ThPwmFreq = 100;
  const int ThPwmMinUs = 1100;
  const int ThPwmMaxUs = 1900;
  // need to adjust for it to make sure it started robustly
  const int ThPwmMiddle = 1450;
  // ThrusterControl class, create my thruster objects
  ThrusterControl ThLF, ThRF, ThLMU, ThRMU, ThLB, ThRB, ThLMD, ThRMD;
};

#endif // THRUSTERFRAMECONTROL_H