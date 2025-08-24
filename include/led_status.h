#ifndef LED_STATUS_H
#define LED_STATUS_H

#include <Arduino.h>

enum LedMode { LED_OFF, LED_ON, LED_BLINK_FAST, LED_BLINK_SLOW, LED_BLINK_OK, LED_BLINK_FAIL };

void initLed(int pin);
void setLedMode(LedMode mode);
void updateLed();

#endif
