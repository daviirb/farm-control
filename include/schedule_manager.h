#pragma once
#include <Arduino.h>

// Estrutura do agendamento
struct ScheduledRelay {
    unsigned long id;
    int pin;
    int hour;
    int minute;
    unsigned long duration;
    int lastFiredMinute;
};

extern const int MAX_SCHEDULED_RELAYS;
extern ScheduledRelay scheduledRelays[];

// Inicialização
void initScheduleManager();

// Controle
void scheduleRelayAtTime(int pin, int hour, int minute, unsigned long duration);
void clearSchedules();
bool deleteScheduleById(int id);
bool updateScheduleById(int id, int pin, int hour, int minute, unsigned long duration);

void listSchedules();

// Loop
void handleScheduledRelays();

// Persistência
void saveSchedules();
void loadSchedules();
