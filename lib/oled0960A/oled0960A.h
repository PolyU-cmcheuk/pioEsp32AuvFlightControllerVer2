#ifndef OLED0960A_H
#define OLED0960A_H

// need to include arduino library for function: Delay(),
#include <Arduino.h>

// display related lib
#include <Adafruit_SSD1306.h>
#include <Adafruit_Sensor.h>

// externally accessable object
extern Adafruit_SSD1306 display;

bool displayInit(int textSize, int rotationMode);
bool displayInit(void);

#endif // OLED0960A_H