

#ifndef EVENT_H
#define EVENT_H

#include <Arduino.h>
#include <stdint.h>

#define NUMBER_EVENT 18
#define SIZE_EVENT 3

struct Event
{
    uint8_t data[SIZE_EVENT];
};

Event createEvent();
Event createEventFromData(uint8_t *data);
void copyEvent(const Event source, Event &destination);

void setBit(Event &event, int position, bool value);
bool getBit(const Event event, int position);

void bitwiseAnd(Event &result, const Event &event1, const Event &event2);
void bitwiseOr(Event &result, const Event &event1, const Event &event2);
void bitwiseXor(Event &result, const Event &event1, const Event &event2);
void bitwiseNot(Event &result, const Event &event);

bool areEqual(const Event &event1, const Event &event2);

void zeroEvent(Event &event);
void oneEvent(Event &event);

void printEvent(const Event event);

#define EVENT_DBAM 17
#define EVENT_LBAM 16
#define EVENT_DBVM 15
#define EVENT_LBVM 14
#define EVENT_DBAZ 13
#define EVENT_LBAZ 12
#define EVENT_F_SUB 11
#define EVENT_F_DESC 10
#define EVENT_SUB 9
#define EVENT_DESC 8
#define EVENT_DVOR 7
#define EVENT_LVOR 6
#define EVENT_COR_OK 5
#define EVENT_COR_N 4
#define EVENT_ITES 3
#define EVENT_VOLUME_OK 2
#define EVENT_VOLUME_N 1
#define EVENT_IPC 0


#endif