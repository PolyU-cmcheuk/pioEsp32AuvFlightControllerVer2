#ifndef TESTREADPOTENTIONMETER_H
#define TESTREADPOTENTIONMETER_H

// need to include arduino library for function: analogRead()
#include <Arduino.h>

// custom dirty code for quick tests
const int testReadPotentionmeterPin = 39;
const int adcRangeMin = 0;
const int adcRangeMax = 4096;
// simple read from ADC pin
unsigned int testReadPotentionmeter(int _adcPin = testReadPotentionmeterPin)
{
  return analogRead(_adcPin);
}
// quick reusable functions
float adcInputNormalizedFloat(float input, float inMin, float inMax, float outMin, float outMax)
{
  return ((input - inMin) * (outMax - outMin) / (inMax - inMin) + outMin);
}
// return normalized result, for bi-direction output [-1.0, 1.0], simply type in the outMin and outMax,
// note: cannot use map function as it needs long as input, not float
float testReadPotentionmeterNomalized(int _adcPin = testReadPotentionmeterPin, float outMin = 0.0, float outMax = 1.0)
{
  return adcInputNormalizedFloat(testReadPotentionmeter(_adcPin), adcRangeMin, adcRangeMax, outMin, outMax);
}

#endif // TESTREADPOTENTIONMETER_H