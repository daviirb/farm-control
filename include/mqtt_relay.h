#ifndef MQTT_RELAY_H
#define MQTT_RELAY_H

#include <Arduino.h>

void initMQTTRelay(const char* brokerIP, uint16_t port);
void setRelayPins(int relayPin, int watterRelayPin);
void handleMQTTRelay();

void activateRelay(int pin, unsigned long duration);
void handleRelayTimer();

void scheduleRelayAtTime(int pin, int targetHour, int targetMinute, unsigned long durationMs);
void handleScheduledRelays();


#endif
