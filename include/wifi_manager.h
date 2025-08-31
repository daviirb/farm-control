#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>
#include "esp_wps.h"

void initWiFi();
void handleBootButtonFactoryReset();

#endif
