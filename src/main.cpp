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

void setup() {
  Serial.begin(115200);
  initLed(LED_PIN);
  initWiFi();
  while (!updateTime()) {
        Serial.println("⏳ Tentando atualizar horário novamente...");
        delay(5000);
  }

  setRelayPins(RELAY_PIN, WATTER_RELAY);

  initMQTTRelay(MQTT_SERVER, MQTT_PORT);
  initScheduleManager();
}

void loop() {
  updateLed();
  handleMQTTRelay();
  handleRelayTimer();
  handleScheduledRelays();
}
