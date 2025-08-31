#include "schedule_manager.h"
#include <Preferences.h>

const int MAX_SCHEDULED_RELAYS = 8;
ScheduledRelay scheduledRelays[MAX_SCHEDULED_RELAYS];
static Preferences prefs;

void initScheduleManager()
{
    prefs.begin("schedules", false);
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
    {
        scheduledRelays[i] = {0, -1, -1, -1, 0, -1};
    }
    loadSchedules();
}
unsigned long generateUniqueId() {
    static unsigned long counter = 0;
    return millis() ^ ((unsigned long)random(1, 1000000)) ^ (++counter);
}

void scheduleRelayAtTime(int pin, int hour, int minute, unsigned long duration) {
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++) {
        if (scheduledRelays[i].pin == -1) {
            scheduledRelays[i] = { generateUniqueId(), pin, hour, minute, duration, -1 };
            Serial.printf("⏱ Agendado: Relay %d às %02d:%02d por %lums (ID: %lu)\n",
                          pin, hour, minute, duration, scheduledRelays[i].id);
            saveSchedules();
            return;
        }
    }
    Serial.println("❌ Nenhum slot disponível para novo agendamento");
}

void clearSchedules()
{
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
    {
        scheduledRelays[i] = {0, -1, -1, -1, 0, -1};
    }
    prefs.clear();
    Serial.println("🗑 Todos os agendamentos foram apagados");
}

void listSchedules()
{
    Serial.println("📋 Lista de agendamentos:");
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
    {
        if (scheduledRelays[i].pin != -1)
        {
            Serial.printf(" - ID %d: Relay %d às %02d:%02d por %lums\n",
                          scheduledRelays[i].id,
                          scheduledRelays[i].pin,
                          scheduledRelays[i].hour,
                          scheduledRelays[i].minute,
                          scheduledRelays[i].duration);
        }
    }
}

extern void activateRelay(int pin, unsigned long duration);

void handleScheduledRelays()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo))
        return;

    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
    {
        ScheduledRelay &s = scheduledRelays[i];
        if (s.pin == -1)
            continue;

        if (timeinfo.tm_hour == s.hour &&
            timeinfo.tm_min == s.minute &&
            s.lastFiredMinute != timeinfo.tm_min)
        {

            activateRelay(s.pin, s.duration);
            s.lastFiredMinute = timeinfo.tm_min;
        }
    }
}

void saveSchedules()
{
    prefs.clear();
    prefs.putInt("count", MAX_SCHEDULED_RELAYS);
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++)
    {
        String key = "s" + String(i);
        if (scheduledRelays[i].pin != -1)
        {
            prefs.putBytes(key.c_str(), &scheduledRelays[i], sizeof(ScheduledRelay));
        }
    }
    Serial.println("💾 Agendamentos salvos na flash");
}

void loadSchedules()
{
    int count = prefs.getInt("count", 0);
    if (count == 0)
    {
        Serial.println("⚠️ Nenhum agendamento encontrado na flash");
        return;
    }
    for (int i = 0; i < count && i < MAX_SCHEDULED_RELAYS; i++)
    {
        String key = "s" + String(i);
        size_t size = sizeof(ScheduledRelay);
        if (prefs.isKey(key.c_str()))
        {
            prefs.getBytes(key.c_str(), &scheduledRelays[i], size);
        }
    }
    Serial.println("📂 Agendamentos carregados da flash");
}

bool deleteScheduleById(int id) {
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++) {
        if (scheduledRelays[i].id == id) {
            scheduledRelays[i] = { 0, -1, -1, 0, 0, -1 };
            saveSchedules();  
            return true;
        }
    }
    return false;
}

bool updateScheduleById(int id, int pin, int hour, int minute, unsigned long duration) {
    for (int i = 0; i < MAX_SCHEDULED_RELAYS; i++) {
        if (scheduledRelays[i].id == id) {
            scheduledRelays[i].pin = pin;
            scheduledRelays[i].hour = hour;
            scheduledRelays[i].minute = minute;
            scheduledRelays[i].duration = duration;
            saveSchedules();  
            Serial.printf("✅ Agendamento ID %d atualizado\n", id);
            return true;
        }
    }
    Serial.printf("❌ Agendamento ID %d não encontrado\n", id);
    return false;
}



