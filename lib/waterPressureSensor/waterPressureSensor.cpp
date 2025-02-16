#include <waterPressureSensor.h>

/**
 * Constructor and destructor
 */
WaterPressureSensor::WaterPressureSensor()
{
}

/**
 * @brief init function for bar30 water pressure sensor
 *
 * @return true
 * @return false
 */
bool WaterPressureSensor::init(void)
{
  // init I2C bus first
  Wire.begin();

  // init bar30 waterpressure sensor
  if (!bar30.init())
  {
    isInitialized = false;
    return isInitialized;
  }

  // after successful initialization of sensor
  bar30.setModel(MS5837::MS5837_30BA);
  bar30.setFluidDensity(waterDensityCoeff);

  return isInitialized;
}

/**
 * @brief
 *
 */
void WaterPressureSensor::read(void)
{
  bar30.read();
}

/**
 * @brief
 *
 * @return float
 */
float WaterPressureSensor::getPressure(void)
{
  return bar30.pressure();
}
/**
 * @brief
 *
 * @return float
 */
float WaterPressureSensor::getTemperature(void)
{
  return bar30.temperature();
}
/**
 * @brief
 *
 * @return float
 */
float WaterPressureSensor::getDepth(void)
{
  return bar30.depth();
}
/**
 * @brief
 *
 * @return float
 */
float WaterPressureSensor::getAltitude(void)
{
  return bar30.altitude();
}