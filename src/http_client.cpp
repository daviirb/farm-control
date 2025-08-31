#include "http_client.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <time.h>
#include <led_status.h>

bool updateTime() {
    HTTPClient http;
    const char* url = "http://worldtimeapi.org/api/timezone/America/Sao_Paulo";

    http.begin(url);
    int httpCode = http.GET();

    if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("✅ Resposta HTTP:");
        Serial.println(payload);
        
        if (updateTimeFromJSON(payload)) {
            setLedMode(LED_BLINK_OK);
            http.end();
            return true;
        } else {
            setLedMode(LED_BLINK_FAIL);
            return false;
        }
    } else {
        Serial.print("❌ Erro na requisição HTTP: ");
        Serial.println(httpCode);
        setLedMode(LED_BLINK_FAIL);
    }

    http.end();
    return false;
}

bool updateTimeFromJSON(String payload) {
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, payload);

    if (!error) {
        long unixtime = doc["unixtime"];
        Serial.printf("⏱ Unixtime recebido: %ld\n", unixtime);

        // Ajusta fuso horário para Brasil (UTC-3)
        setenv("TZ", "BRT3", 1);  // POSIX: UTC-3
        tzset();

        struct timeval tv = { .tv_sec = unixtime, .tv_usec = 0 };
        settimeofday(&tv, nullptr);

        struct tm timeinfo;
        if (getLocalTime(&timeinfo)) {
            Serial.printf("Hora LOCAL: %02d:%02d:%02d\n",
                          timeinfo.tm_hour,
                          timeinfo.tm_min,
                          timeinfo.tm_sec);
        } else {
            Serial.println("❌ Falha ao obter hora local");
        }

        return true;
    } else {
        Serial.println("❌ Erro ao parsear JSON");
        return false;
    }
}
