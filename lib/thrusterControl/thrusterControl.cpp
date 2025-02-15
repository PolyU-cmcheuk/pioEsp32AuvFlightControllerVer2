#include <thrusterControl.h>

/**
 * @brief Construct a new Thruster Control:: Thruster Control object
 *
 */
ThrusterControl::ThrusterControl(int pwmPin)
{
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  // configure parameters
  _pwmPin = pwmPin;
  // set parameters to hardware
  this->pwm.setPeriodHertz(_pwmFreq);
  this->pwm.attach(_pwmPin, _pwmMinUs, _pwmMaxUs);
  // write pwm as initial value
  this->pwm.writeMicroseconds(_pwmMiddle);
}
/**
 * @brief Construct a new Thruster Control:: Thruster Control object
 *
 * @param pwmPin
 * @param pwmFreq
 * @param pwmMinUs
 * @param pwmMaxUs
 */
ThrusterControl::ThrusterControl(int pwmPin, int pwmFreq, int pwmMinUs, int pwmMaxUs, int pwmMiddleUs, SpinDirection spinDirection)
{
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  // configure parameters
  _pwmPin = pwmPin;
  this->setPwmParameters(pwmFreq, pwmMinUs, pwmMaxUs);
  // set parameters to hardware
  this->pwm.setPeriodHertz(_pwmFreq);
  this->pwm.attach(_pwmPin, _pwmMinUs, _pwmMaxUs);

  // it will set the mode inside or else keep the default
  if (isValidSpinDirection(spinDirection))
  {
  };
  // write pwm as initial value according to the mode
  _pwmMiddle = pwmMiddleUs;
  this->pwm.writeMicroseconds(_pwmMiddle);
}

/**
 * @brief check if the input is valid or not, set the mode if valid
 *
 * @param spinDirection
 * @return true
 * @return false
 */
bool ThrusterControl::isValidSpinDirection(SpinDirection spinDirection)
{
  switch (spinDirection)
  {
  case CW:
  case CCW:
    _spinDirection = spinDirection;
    return true;
  default:
    return false;
  }
}

/**
 * @brief quick reusable function for mapping floating input and output by linear interpolation
 *
 * @param input
 * @param inMin
 * @param inMax
 * @param outMin
 * @param outMax
 * @return float
 */
float ThrusterControl::mapFloat(float input, float inMin, float inMax, float outMin, float outMax)
{
  return ((input - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
}
/**
 * @brief function for updating all parameters after configuring any one of them
 *
 * @return true if all parameters have been updated
 * @return false (usually this will not be problematic as all setta functions should have self checking functions)
 */
bool ThrusterControl::updateParameters(void)
{
  // only need to update the following parameters inside class
  _pwmRange = _pwmMaxUs - _pwmMinUs;
  // update to hardware
  this->pwm.attach(_pwmPin, _pwmMinUs, _pwmMaxUs);
  return true;
}

/**
 * @brief set pwm frequency, with error checking for input range [0Hz 400Hz]
 *
 * @param pwmFreq
 * @return true if correctly set
 * @return false if input parameter out of range
 */
bool ThrusterControl::setPwmFreq(int pwmFreq)
{
  // error checking
  if (pwmFreq < 0 || pwmFreq > 400)
  {
    return false;
  }
  // allow update
  _pwmFreq = pwmFreq;
  // update other parameters if necessary
  this->updateParameters();
  // update to hardware
  this->pwm.setPeriodHertz(_pwmFreq);
  return true;
}

/**
 * @brief set pwmMinUs
 *
 * @param pwmMinUs
 * @return true
 * @return false
 */
bool ThrusterControl::setPwmMinUs(int pwmMinUs)
{
  // error checking: constants come from popular ESC controllable ranges
  if (pwmMinUs < 500 || pwmMinUs > 2500)
  {
    return false;
  }
  // allow update
  _pwmMinUs = pwmMinUs;
  // update other parameters if necessary
  this->updateParameters();

  return true;
}

/**
 * @brief set pwmMaxUs
 *
 * @param pwmMaxUs
 * @return true
 * @return false
 */
bool ThrusterControl::setPwmMaxUs(int pwmMaxUs)
{
  // error checking: constants come from popular ESC controllable ranges
  if (pwmMaxUs < 500 || pwmMaxUs > 2500)
  {
    return false;
  }
  // allow update
  _pwmMaxUs = pwmMaxUs;
  // update other parameters if necessary
  this->updateParameters();

  return true;
}
/**
 * @brief simple setup important parameters at once
 *
 * @param pwmFreq
 * @param pwmMinUs
 * @param pwmMaxUs
 * @return true
 * @return false
 */
bool ThrusterControl::setPwmParameters(int pwmFreq, int pwmMinUs, int pwmMaxUs)
{
  // set parameters, return false if any errors
  if (!this->setPwmFreq(pwmFreq))
    return false;
  if (!this->setPwmMinUs(pwmMinUs))
    return false;
  if (!this->setPwmMaxUs(pwmMaxUs))
    return false;
  // update parameters and send to hardware
  if (!this->updateParameters())
    return false;
  // return true if there is no errors from all above steps
  return true;
}
/**
 * @brief set pwm values, with boundary check,
 *
 * @param pwmValueUs
 * @return true if within bound
 * @return false if out of bound
 */
bool ThrusterControl::setPwmValue(int pwmValueUs)
{
  // error range check, return false, and bound the input by the Min/Max value
  if (pwmValueUs < _pwmMinUs)
  {
    _pwmValueUs = _pwmMinUs;
    this->pwm.writeMicroseconds(_pwmValueUs);
    return false;
  }
  else if (pwmValueUs > _pwmMaxUs)
  {
    _pwmValueUs = _pwmMaxUs;
    this->pwm.writeMicroseconds(_pwmValueUs);
    return false;
  }

  // allow update
  _pwmValueUs = pwmValueUs;
  this->pwm.writeMicroseconds(_pwmValueUs);
  return true;
}
/**
 * @brief set pwm values with normalized input, with boundary check,
 *
 * @param pwmNormalized
 * @return true
 * @return false
 */
bool ThrusterControl::setPwmValueNormalized(float pwmNormalized)
{
  // variables for calculating correct pwmUs value from pwmNormalized
  // note: since the middle value is manually set, the slopes for +ve side and -ve side may be different.
  float inputMin = -1.0, inputMax = 1.0, inputMiddle = 0.0;

  // error range check, return false, and bound the input by the Min/Max value
  if (pwmNormalized < inputMin)
  {
    pwmNormalized = inputMin;
  }
  else if (pwmNormalized > inputMax)
  {
    pwmNormalized = inputMax;
  }

  // adjust for CCW orientations, CW:+ve, CCW:-ve
  if (_spinDirection == CCW)
  {
    pwmNormalized = -pwmNormalized;
  }

  // compute pwmValueUs, but need to compute in two rates
  if (pwmNormalized < inputMiddle)
  {
    _pwmValueUs = mapFloat(pwmNormalized, inputMin, inputMiddle, _pwmMinUs, _pwmMiddle);
  }
  else
  {
    _pwmValueUs = mapFloat(pwmNormalized, inputMiddle, inputMax, _pwmMiddle, _pwmMaxUs);
  }

  // actual update to hardware
  this->pwm.writeMicroseconds(_pwmValueUs);

  return true;
}
/**
 * @brief get pwm value
 *
 * @return int
 */
int ThrusterControl::getPwmValue()
{
  return _pwmValueUs;
}
