#include <oled0960A.h>

// object for display
Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);
bool displayEnable = false;
int textSize = 1;

/**
 * @brief simple displayInit() function
 *
 * @param textSize  range from 1 to ??
 * @param rotationMode possible choice: 0,1,2,3, representing different orientations
 * @return true
 * @return false
 */
bool displayInit(int textSize, int rotationMode)
{
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    // may include error print function via Serial.print() if necessary
    displayEnable = false;
    return false;
  }

  display.display();
  delay(500);
  display.setTextSize(textSize);
  display.setTextColor(WHITE);
  display.setRotation(rotationMode);

  displayEnable = true;
  return true;
}
/**
 * @brief overloaded displayInit() function with default inputs
 *
 * @return true
 * @return false
 */
bool displayInit(void)
{
  // setting default mode
  return displayInit(1, 0);
}
