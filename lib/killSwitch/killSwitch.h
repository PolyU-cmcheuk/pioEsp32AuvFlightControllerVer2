#ifndef KILLSWITCH_H
#define KILLSWITCH_H

// need to include arduino library for function: analogRead()
#include <Arduino.h>

// kill switch detection with optocoupla
const int killSwitch12vPin = 36; // GPIO36
unsigned int killSwitch12vAdcValue = 0;
unsigned int killSwitch12vAdcThreshold = 1000; // if lower than this value (resistor pull ground), consider as killswitch activated
bool isKillSwitchActivated(int _killSwitch12vPin = killSwitch12vPin)
{
  if (analogRead(_killSwitch12vPin) < killSwitch12vAdcThreshold)
    return true;
  else
    return false;
}

#endif // KILLSWITCH_H