
#ifndef CALENDAR_EVENT_H
#define CALENDAR_EVENT_H

#include <Arduino.h>

class CalendarEvent
{
    public:
    String eventTitle;
    // Represent an event's start and end as minutes since midnight for simplicity
    // Any information about what day the event takes place will be handeled externally
    uint16_t startMinsSinceMidnight;
    uint16_t endMinsSinceMidnight;

    CalendarEvent();

    CalendarEvent(String title, int startHour, int startMin, int endHour, int endMin);
    void clear();
    void set(String title, int startHour, int startMin, int endHour, int endMin);
    int getEventMins();
    int getStartTimeMins();
    int getEndTimeMins();
};

#endif