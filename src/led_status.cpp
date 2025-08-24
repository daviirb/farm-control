#include "led_status.h"

static int ledPin;
static LedMode currentMode = LED_OFF;
static unsigned long lastBlink = 0;
static bool ledState = false;

void initLed(int pin)
{
  ledPin = pin;
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
}

void setLedMode(LedMode mode)
{
  currentMode = mode;
}

void updateLed()
{
  unsigned long now = millis();

  switch (currentMode)
  {
  case LED_ON:
    digitalWrite(ledPin, HIGH);
    break;
  case LED_OFF:
    digitalWrite(ledPin, LOW);
    break;
  case LED_BLINK_FAST:
    if (now - lastBlink >= 200)
    {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      lastBlink = now;
    }
    break;
  case LED_BLINK_SLOW:
    if (now - lastBlink >= 1000)
    {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      lastBlink = now;
    }
    break;
  case LED_BLINK_FAIL:
  if (now - lastBlink >= 2000)
    {
      ledState = !ledState;
      digitalWrite(ledPin, ledState);
      lastBlink = now;
    }
    break;
  case LED_BLINK_OK:
  {
    static int step = 0;
    static int blinkCount = 0;
    static int part = 0;
    static bool ledOn = false;
    static unsigned long finishedTime = 0;

    unsigned long now = millis();

    if (step == 0)
    {
      if (now - lastBlink >= 200)
      {
        ledOn = !ledOn;
        digitalWrite(ledPin, ledOn);
        lastBlink = now;
        if (!ledOn)
          blinkCount++;
        if (blinkCount >= 3)
        {
          step = 1;
          blinkCount = 0;
        }
      }
    }
    else if (step == 1)
    {
      if (now - lastBlink >= 500)
      {
        step = 2;
        part = 0;
        ledOn = false;
        digitalWrite(ledPin, LOW);
        lastBlink = now;
      }
    }
    else if (step == 2)
    {
      unsigned long duration = (part == 1 ? 600 : 200);
      if (now - lastBlink >= duration)
      {
        ledOn = !ledOn;
        digitalWrite(ledPin, ledOn);
        lastBlink = now;
        if (!ledOn)
          part++;
        if (part > 2)
        {
          step = 3;
          finishedTime = now;
          digitalWrite(ledPin, LOW);
        }
      }
    }
    else if (step == 3)
    {
      if (now - finishedTime >= 1000)
      {
        setLedMode(LED_ON);
      }
    }
  }
  break;
  }
}
