#ifndef WATERPRESSURESENSOR_H
#define WATERPRESSURESENSOR_H

// need to include arduino library for function: Delay(),
#include <Arduino.h>
// Bar30 sensor needs I2C
#include <Wire.h>
// bar30 sensor contributed lib
#include "MS5837.h"

class WaterPressureSensor
{
public:
  // constructor and desctructor
  WaterPressureSensor();
  ~WaterPressureSensor() {};

  // function prototypes
  bool init(void);
  void read(void);

  // getta functions
  float getPressure(void);    // unit = mbar
  float getTemperature(void); // unit = degC
  float getDepth(void);       // unit = m (below water surface)
  float getAltitude(void);    // ubit = m (above mean sea level)

private:
  const int waterDensityCoeff = 997; // kg/m^3 (freshwater, 1029 for seawater)
  MS5837 bar30;
  bool isInitialized = false;
};

#endif // WATERPRESSURESENSOR_H