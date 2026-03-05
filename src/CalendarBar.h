
#ifndef CALENDAR_BAR_H
#define CALENDAR_BAR_H

#include <Arduino.h>
#include <time.h>

#include "CalendarEvent.h"
#include "SDController.h"
#include "ScreenController.h"

#define EVENT_BUFFER_SIZE 6

class CalendarBar
{

    private:
    int barWidth        = 50; // How wide the bar is
    int eventSpacing    = 3;  // Spacing between each event on the bar
    int screenMargin    = 20; // Padding between the bar and screen edges
    int pointerLength   = 10; // Length of the pointer
    int barWorkArea     = 0;  // Maximum size for the bar
    int dayTotalMinutes = 0;  // Total number of minutes per day

    public:
    ScreenController *screenCont;
    CalendarEvent calendarEvents[EVENT_BUFFER_SIZE];
    int eventCount = 0;
    const int dayStartTime[2] = {7, 0}; // 7:00am
    const int dayEndTime[2] = {23, 0};  // 11:00pm

    // Pointer to the image cache
    UBYTE *BlackImage = nullptr;

    struct tm *timeinfo;

    SDController *sdCont;

    CalendarBar();
    void initialize();
    void initCalendarBarEvents();
    void drawCalendarBar();
    void drawPointer();

};

#endif