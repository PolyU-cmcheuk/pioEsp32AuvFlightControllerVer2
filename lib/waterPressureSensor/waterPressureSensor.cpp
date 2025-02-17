#include "MS5837.h"
#include <waterPressureSensor.h>

// copied from MS5837.cpp here in order to use some of the constants
#define MS5837_ADDR 0x76
#define MS5837_RESET 0x1E
#define MS5837_ADC_READ 0x00
#define MS5837_PROM_READ 0xA0
#define MS5837_CONVERT_D1_8192 0x4A
#define MS5837_CONVERT_D2_8192 0x5A

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
 * @brief non blocking read functions, need to uncomment the "private:" keywords to use some of the variables inside
 *
 *
 */
void WaterPressureSensor::nonBlockReadD1Request(void)
{
  // Request D1 conversion
  Wire.beginTransmission(MS5837_ADDR);
  Wire.write(MS5837_CONVERT_D1_8192);
  Wire.endTransmission();

  // then need to wait for about 20ms before next read
  // usually we can let the MCU run other codes which we are sure that
  // they will last for sufficient time for data conversion
}
void WaterPressureSensor::nonBlockReadD1Receive(void)
{
  // Request D1 data
  Wire.beginTransmission(MS5837_ADDR);
  Wire.write(MS5837_ADC_READ);
  Wire.endTransmission();

  Wire.requestFrom(MS5837_ADDR, 3);
  bar30.D1 = 0;
  bar30.D1 = Wire.read();
  bar30.D1 = (bar30.D1 << 8) | Wire.read();
  bar30.D1 = (bar30.D1 << 8) | Wire.read();

  // then need to wait for about 20ms before next read
  // usually we can let the MCU run other codes which we are sure that
  // they will last for sufficient time for data conversion
}
void WaterPressureSensor::nonBlockReadD2Request(void)
{
  // Request D2 conversion
  Wire.beginTransmission(MS5837_ADDR);
  Wire.write(MS5837_CONVERT_D2_8192);
  Wire.endTransmission();

  // then need to wait for about 20ms before next read
  // usually we can let the MCU run other codes which we are sure that
  // they will last for sufficient time for data conversion
}
void WaterPressureSensor::nonBlockReadD2Receive(void)
{
  // request D2 data
  Wire.beginTransmission(MS5837_ADDR);
  Wire.write(MS5837_ADC_READ);
  Wire.endTransmission();

  Wire.requestFrom(MS5837_ADDR, 3);
  bar30.D2 = 0;
  bar30.D2 = Wire.read();
  bar30.D2 = (bar30.D2 << 8) | Wire.read();
  bar30.D2 = (bar30.D2 << 8) | Wire.read();

  bar30.calculate();

  // then need to wait for about 20ms before next read
  // usually we can let the MCU run other codes which we are sure that
  // they will last for sufficient time for data conversion
}
void WaterPressureSensor::nonBlockRead(void)
{
  switch (nonBlockReadNextTask)
  {
  case D1Request:
    nonBlockReadD1Request();
    nonBlockReadNextTask = D1Receive; // 1
    break;
  case D1Receive:
    nonBlockReadD1Receive();
    nonBlockReadNextTask = D2Request; // 2
    break;
  case D2Request:
    nonBlockReadD2Request();
    nonBlockReadNextTask = D2Receive; // 3
    break;
  case D2Receive:
    nonBlockReadD2Receive();
    nonBlockReadNextTask = D1Request; // 0
    break;
  default:
    nonBlockReadNextTask = D1Request; // 0
    break;
  }
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