#include <Arduino.h>
#include "wifi_manager.h"
#include "led_status.h"
#include "http_client.h"
#include "mqtt_relay.h"
#include "schedule_manager.h"
#include "config.h"

#define LED_PIN 2
#define RELAY_PIN 26
#define WATTER_RELAY 27
#define BUTTON_PIN 0

void setup()
{
  Serial.begin(115200);
  initLed(LED_PIN);
  initWiFi();

  if (WiFi.status() == WL_CONNECTED)
  {
    unsigned long start = millis();
    bool ok = false;

    while (!ok && millis() - start < 5000)
    {
      ok = updateTime();
      if (!ok)
      {
        delay(200);
      }
    }

    if (ok)
    {
      Serial.println("⏰ Hora atualizada com sucesso!");
    }
    else
    {
      Serial.println("⚠️ Falha ao atualizar hora após conectar WiFi");
    }
  }

  setRelayPins(RELAY_PIN, WATTER_RELAY);

  initMQTTRelay(MQTT_SERVER, MQTT_PORT);
  initScheduleManager();
}

void loop()
{
  updateLed();
  handleBootButtonFactoryReset();
  if (WiFi.status() == WL_CONNECTED)
  {
    handleMQTTRelay();
    handleRelayTimer();
    handleScheduledRelays();
  }
}
