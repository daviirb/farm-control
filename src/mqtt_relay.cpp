#include "mqtt_relay.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include "schedule_manager.h"
#include <ArduinoJson.h>

// --- Objetos MQTT ---
static WiFiClient espClient;
static PubSubClient client(espClient);

// --- Relays físicos ---
static int _relayPin = -1;
static int _watterRelay = -1;

// --- Timer dos relays imediatos ---
struct RelayTimer
{
    int pin = -1;
    unsigned long endTime = 0;
    bool active = false;
};

static const int MAX_RELAYS = 2;
static RelayTimer relayTimers[MAX_RELAYS];

// --- Config do Broker ---
static const char *_brokerIP = nullptr;
static uint16_t _brokerPort = 1883;

// ======================================================
// FUNÇÕES AUXILIARES
// ======================================================
void activateRelay(int pin, unsigned long duration)
{
    for (int i = 0; i < MAX_RELAYS; i++)
    {
        if (!relayTimers[i].active)
        {
            relayTimers[i].pin = pin;
            relayTimers[i].endTime = millis() + duration;
            relayTimers[i].active = true;
            digitalWrite(pin, HIGH);
            Serial.printf("⚡ Relay %d ativado por %lums\n", pin, duration);
            return;
        }
    }
    Serial.println("❌ Nenhum slot disponível para ativar outro relay");
}

void handleRelayTimer()
{
    unsigned long now = millis();
    for (int i = 0; i < MAX_RELAYS; i++)
    {
        if (relayTimers[i].active && now >= relayTimers[i].endTime)
        {
            digitalWrite(relayTimers[i].pin, LOW);
            Serial.printf("⚡ Relay %d desligado\n", relayTimers[i].pin);
            relayTimers[i].active = false;
        }
    }
}

static void callback(char *topic, byte *payload, unsigned int length)
{
    String t = String(topic);

    String msg;
    for (unsigned int i = 0; i < length; i++)
        msg += (char)payload[i];
    msg.trim();

    Serial.print("📩 Tópico: ");
    Serial.println(t);
    Serial.print("📩 Payload: ");
    Serial.println(msg);

    int duration = 0;
    String command = msg;

    int colonIndex = msg.indexOf(':');
    if (colonIndex != -1)
    {
        command = msg.substring(0, colonIndex);
        duration = msg.substring(colonIndex + 1).toInt();
    }

    // ---- COMANDOS DIRETOS ----
    if (t == "farm/relay")
{
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, msg);

    if (error)
    {
        Serial.print("❌ Erro ao parsear JSON em farm/relay: ");
        Serial.println(error.c_str());
        return;
    }

    int pin = doc["pin"] | -1;
    String command = doc["command"] | "";
    unsigned long dur = doc["duration"] | 0;

    if(pin < 0 || command == "") {
        Serial.println("❌ Dados inválidos no JSON");
        return;
    }

    Serial.printf("📥 Comando para Relay - Pin: %d, Command: %s, Duration: %lu\n",
                  pin, command.c_str(), dur);

    if (command.equalsIgnoreCase("ON") && dur > 0)
    {
        activateRelay(pin, dur);
    }
    else
    {
        digitalWrite(pin, command.equalsIgnoreCase("ON") ? HIGH : LOW);
        Serial.printf("⚡ Relay %d definido como %s\n", pin, command.c_str());
    }
}


    // ---- AGENDAMENTOS (repasse ao schedule_manager) ----
    else if (t == "farm/schedule/add")
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, msg);

        if (!error)
        {
            int pin = doc["pin"];
            int hour = doc["hour"];
            int minute = doc["minute"];
            int duration = doc["duration"];

            scheduleRelayAtTime(pin, hour, minute, duration);
        }
        else
        {
            Serial.println("❌ Erro ao parsear JSON do agendamento");
        }
    }
    else if (t == "farm/schedule/clear")
    {
        clearSchedules();
    }
    else if (t == "farm/schedule/deleteById")
    {
        StaticJsonDocument<64> doc;
        DeserializationError error = deserializeJson(doc, msg);
        if (error)
        {
            Serial.println("❌ JSON inválido para deleteById");
            return;
        }
        int id = doc["id"];
        if (deleteScheduleById(id))
        {
            Serial.printf("✅ Agendamento ID %d deletado\n", id);
        }
        else
        {
            Serial.printf("❌ Agendamento ID %d não encontrado\n", id);
        }
    }

    else if (t == "farm/schedule/updateById")
    {
        StaticJsonDocument<200> doc;
        DeserializationError error = deserializeJson(doc, msg);

        if (error)
        {
            Serial.println("❌ JSON inválido para updateById");
            return;
        }

        int id = doc["id"];
        int pin = doc.containsKey("pin") ? doc["pin"] : -1;
        int hour = doc.containsKey("hour") ? doc["hour"] : -1;
        int minute = doc.containsKey("minute") ? doc["minute"] : -1;
        long duration = doc.containsKey("duration") ? doc["duration"] : -1;

        updateScheduleById(id, pin, hour, minute, duration);
    }

    else if (t == "farm/schedule/list")
    {
        String response = "[";
        bool first = true;

        for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
        {
            if (scheduledRelays[i].pin != -1)
            {
                if (!first)
                    response += ",";

                response += "{";
                response += "\"id\":" + String(scheduledRelays[i].id) + ",";
                response += "\"pin\":" + String(scheduledRelays[i].pin) + ",";
                response += "\"hour\":" + String(scheduledRelays[i].hour) + ",";
                response += "\"minute\":" + String(scheduledRelays[i].minute) + ",";
                response += "\"duration\":" + String(scheduledRelays[i].duration);
                response += "}";

                first = false;
            }
        }
        response += "]";

        Serial.println("📋 Lista de agendamentos:");
        Serial.println(response);

        client.publish("farm/schedule/list/response", response.c_str(), true);
    }
}

static unsigned long lastReconnectAttempt = 0;

static void reconnect()
{
   if (!client.connected())
    {
        unsigned long now = millis();
        if (now - lastReconnectAttempt > 5000)
        {
            lastReconnectAttempt = now;
            Serial.print("🔌 Tentando reconectar MQTT...");
            if (client.connect("ESP32Client"))
            {
                Serial.println("✅ Reconectado!");
                client.subscribe("farm/relay");
                client.subscribe("farm/watter");
                client.subscribe("farm/schedule/add");
                client.subscribe("farm/schedule/clear");
                client.subscribe("farm/schedule/list");
                client.subscribe("farm/schedule/deleteById");
                client.subscribe("farm/schedule/updateById");
            }
            else
            {
                Serial.print("❌ Falha rc=");
                Serial.println(client.state());
            }
        }
    }
}

void initMQTTRelay(const char *brokerIP, uint16_t port)
{
    _brokerIP = brokerIP;
    _brokerPort = port;
    client.setServer(_brokerIP, _brokerPort);
    client.setBufferSize(1024);
    client.setCallback(callback);
}

void setRelayPins(int relayPin, int watterRelayPin)
{
    _relayPin = relayPin;
    _watterRelay = watterRelayPin;
    pinMode(_relayPin, OUTPUT);
    pinMode(_watterRelay, OUTPUT);
    digitalWrite(_relayPin, LOW);
    digitalWrite(_watterRelay, LOW);

    Serial.print("⚡ Relay Pin setado em: ");
    Serial.println(_relayPin);
    Serial.print("⚡ Watter Relay Pin setado em: ");
    Serial.println(_watterRelay);
}

void handleMQTTRelay()
{
    if (!client.connected())
        reconnect();
    client.loop();
}
