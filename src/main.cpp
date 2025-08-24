#include <Arduino.h>
#include "wifi_manager.h"
#include "led_status.h"
#include "http_client.h"



#define LED_PIN 2

void setup() {
  Serial.begin(115200);
  initLed(LED_PIN);
  initWiFi();
  while (!updateTime()) {
        Serial.println("⏳ Tentando atualizar horário novamente...");
        delay(5000);
    }
}

void loop() {
  updateLed();
}
