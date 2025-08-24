#include "wifi_manager.h"
#include "led_status.h"
#include "http_client.h"
#include "esp_wps.h"

static esp_wps_config_t wps_config;

void WiFiEvent(WiFiEvent_t event) {
    switch(event) {

        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("[WiFi] STA iniciado. Checando WPS...");
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            Serial.print("[WiFi] ✅ Conectado! IP: ");
            Serial.println(WiFi.localIP());
            setLedMode(LED_ON);
            updateTime();
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            Serial.print("[WiFi] ⚠️ Desconectado. Status: ");
            setLedMode(LED_BLINK_SLOW);
            break;

        case ARDUINO_EVENT_WPS_ER_SUCCESS:
            Serial.println("[WiFi] 🔑 WPS OK, credenciais recebidas!");
            esp_wifi_wps_disable();
            WiFi.begin();
            break;

        case ARDUINO_EVENT_WPS_ER_FAILED:
        case ARDUINO_EVENT_WPS_ER_TIMEOUT:
            Serial.println("[WiFi] ❌ WPS falhou/timeout. Tentando novamente...");
            esp_wifi_wps_disable();
            esp_wifi_wps_enable(&wps_config);
            esp_wifi_wps_start(0);
            break;

        default:
            break;
    }
}

void initWiFi() {
    memset(&wps_config, 0, sizeof(wps_config));
    wps_config.wps_type = WPS_TYPE_PBC;
    strcpy(wps_config.factory_info.manufacturer, "ESPRESSIF");
    strcpy(wps_config.factory_info.model_number, "ESP32");
    strcpy(wps_config.factory_info.model_name, "ESPRESSIF_IOT");
    strcpy(wps_config.factory_info.device_name, "ESP32_FEEDER");

    WiFi.mode(WIFI_STA);
    WiFi.persistent(true);
    WiFi.setAutoReconnect(true);
    WiFi.onEvent(WiFiEvent);

    Serial.println("[WiFi] Tentando conectar com credenciais salvas...");
    WiFi.begin();

    unsigned long start = millis();
    while(WiFi.status() != WL_CONNECTED && millis() - start < 5000){
        delay(500);
        Serial.print(".");
    }

    if(WiFi.status() == WL_CONNECTED){
        Serial.println("\n[WiFi] ✅ Conectado usando credenciais salvas!");
        setLedMode(LED_ON);
        updateTime();
    } else {
        Serial.println("\n[WiFi] ⚠️ Falha com credenciais salvas. Limpando e ativando WPS...");
        
        WiFi.disconnect(true, true); 
        delay(1000);
        WiFi.mode(WIFI_STA);
        setLedMode(LED_BLINK_FAST);
        esp_wifi_wps_enable(&wps_config);
        esp_wifi_wps_start(0);
    }
}
