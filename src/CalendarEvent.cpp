
#include "CalendarEvent.h"

CalendarEvent::CalendarEvent()
{
    this->eventTitle = "";
    this->startMinsSinceMidnight = 0;
    this->endMinsSinceMidnight = 0;
}

CalendarEvent::CalendarEvent(String title, int startHour, int startMin, int endHour, int endMin)
{
    this->eventTitle = title;
    this->startMinsSinceMidnight = startHour * 60 + startMin;
    this->endMinsSinceMidnight = endHour * 60 + endMin;
}

void CalendarEvent::clear()
{
    this->eventTitle = "";
    this->startMinsSinceMidnight = 0;
    this->endMinsSinceMidnight = 0;
}

void CalendarEvent::set(String title, int startHour, int startMin, int endHour, int endMin)
{
    this->eventTitle = title;
    this->startMinsSinceMidnight = startHour * 60 + startMin;
    this->endMinsSinceMidnight = endHour * 60 + endMin;
}

int CalendarEvent::getEventMins()
{
    int temp = this->endMinsSinceMidnight - this->startMinsSinceMidnight;
    return temp;
}

int CalendarEvent::getStartTimeMins()
{
    return this->startMinsSinceMidnight;
}

int CalendarEvent::getEndTimeMins()
{
    return this->endMinsSinceMidnight;
}