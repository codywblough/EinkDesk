
#include "CalendarBar.h"

CalendarBar::CalendarBar()
{

}

void CalendarBar::initialize()
{
    // Calculate the total number of minutes per day
    dayTotalMinutes = dayEndTime[0] * 60 + dayEndTime[1] - (dayStartTime[0] * 60 + dayStartTime[1]);
    // Calculate the total bar area to work with
    //                Screen height    - pad btwn top/bot
    barWorkArea = this->screenCont->getDisplayHeight() - 2 * screenMargin; // Note there will be some error at the end but it is whatever
}

// This would be like the 'get events each day' type thing
void CalendarBar::initCalendarBarEvents()
{
    Serial.println("Initializing calendar bar events");
    this->eventCount = 0; // Reset eventCount

    // Clear out the event buffer
    for (int i = 0; i < EVENT_BUFFER_SIZE; i++)
    {
        this->calendarEvents[i].clear();
    }
    // Check day of week to pick correct file
    switch (this->timeinfo->tm_wday)
    {
        case 0:
            this->sdCont->openFile("/calendar/sunday.txt");
            Serial.println("Opening Sunday file");
            break;
        case 1:
            this->sdCont->openFile("/calendar/monday.txt");
            Serial.println("Opening Monday file");
            break;
        case 2:
            this->sdCont->openFile("/calendar/tuesday.txt");
            Serial.println("Opening Tuesday file");
            break;
        case 3:
            this->sdCont->openFile("/calendar/wednesday.txt");
            Serial.println("Opening Wednesday file");
            break;
        case 4:
            this->sdCont->openFile("/calendar/thursday.txt");
            Serial.println("Opening Thursday file");
            break;
        case 5:
            this->sdCont->openFile("/calendar/friday.txt");
            Serial.println("Opening Friday file");
            break;
        case 6:
            this->sdCont->openFile("/calendar/saturday.txt");
            Serial.println("Opening Saturday file");
            break;
        default:
            Serial.println("Error: Default case reached for tm_wday");
            break;
    }
    // Check if we are unable to open file
    if (!this->sdCont->openedFile)
    {
        Serial.println("Failed to open file");
        // Probably needs something else in here
        return;
    }

    String title;
    String line;
    int startHour;
    int startMin;
    int endHour;
    int endMin;
    
    while(this->sdCont->openedFile.available() != 0)
    {
        // Format in file is:
        // title
        // startHour:startMin-endHour:endMin
        // Read in the two lines
        title = this->sdCont->openedFile.readStringUntil('\n');
        line = this->sdCont->openedFile.readStringUntil('\n');
        // Separate the 2nd line into the data
        startHour = line.substring(0, line.indexOf(':')).toInt();
        startMin = line.substring(line.indexOf(':') + 1, line.indexOf('-')).toInt();
        endHour = line.substring(line.indexOf('-') + 1, line.lastIndexOf(':')).toInt();
        endMin = line.substring(line.lastIndexOf(':') + 1).toInt();

        Serial.println("Found event:");
        Serial.println(title);
        Serial.printf("%d:%d-%d:%d\n", startHour, startMin, endHour, endMin);
        this->calendarEvents[this->eventCount].set(title, startHour, startMin, endHour, endMin);
        this->eventCount++;
    }
    Serial.printf("%d events found\n", this->eventCount);
    this->sdCont->closeFile();

}

void CalendarBar::drawCalendarBar()
{
    // Bar drawing variables
    int eventWidth = 0; // Temporary storage for pixel width of an event
    int currentStartPoint = screenMargin;

    Serial.println("\ndrawCalendarBar()");
    Serial.println(barWorkArea);
    Serial.println(dayTotalMinutes);
    if (this->eventCount > 0)
    {
        // Draw a rectangle from start of day to the first event
        eventWidth = int((double)(calendarEvents[0].getStartTimeMins() - (dayStartTime[0] * 60 + dayStartTime[1])) / dayTotalMinutes * barWorkArea); // calculate bar width
        if (eventWidth > 0) // Check for the first bar being 0 pixels wide
        {
            Serial.print("Starter: ");
            Serial.println(eventWidth);
            this->screenCont->drawRectangle(screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth - eventSpacing, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
            currentStartPoint += eventWidth + eventSpacing; // Shift the start point
        }

        for (int i = 0; i < this->eventCount; i++)
        {
            // Draw rectangle for event
            eventWidth = int((double)(calendarEvents[i].getEventMins()) / dayTotalMinutes * barWorkArea); // Calculate bar width
            Serial.print("Event: ");
            Serial.println(eventWidth);
            this->screenCont->drawRectangle(screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth - eventSpacing, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
            currentStartPoint += eventWidth + eventSpacing; // Shift the start point

            // Create a filler rectangle between current event and next event
            if (i < eventCount - 1)
            {
                eventWidth = int((double)(calendarEvents[i + 1].getStartTimeMins() - calendarEvents[i].getEndTimeMins()) / dayTotalMinutes * barWorkArea);
                Serial.print("Filler: ");
                Serial.println(eventWidth);
                Serial.printf("%d %d %d %d\n", screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth);
                this->screenCont->drawRectangle(screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth - eventSpacing, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
                currentStartPoint += eventWidth + eventSpacing; // Shift the start point
            }
        }
        // Draw one last rectangle that goes from the last event to the set end time for the day
        eventWidth = int((double)((dayEndTime[0] * 60 + dayEndTime[1]) - calendarEvents[eventCount - 1].getEndTimeMins()) / dayTotalMinutes * barWorkArea);
        Serial.print("Closer: ");
        Serial.println(eventWidth);
        if (eventWidth > 0)
            this->screenCont->drawRectangle(screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth - eventSpacing, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
        // No need to shift starting point again
    }
    else
    {
        // If there are no events in the day, draw one big rectangle for no event
        eventWidth = barWorkArea;
        this->screenCont->drawRectangle(screenMargin, currentStartPoint, screenMargin + barWidth, currentStartPoint + eventWidth, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    }    

    // Draw hour ticks
    for (int i = dayStartTime[0]; i <= dayEndTime[0]; i++) // Note that this loop assumes start time is a round XX:00
    {
        int tickHeight = screenMargin + int((double)(i * 60 - dayStartTime[0] * 60) / dayTotalMinutes * barWorkArea);
        // Draw ticks on opposite side from pointer
        //this->screenCont->drawLine(screenMargin + barWidth + 5, tickHeight, screenMargin + barWidth + 5 + 10, tickHeight, BLACK, DOT_PIXEL_1X1, LINE_STYLE_SOLID);
        // Draw ticks on same side as pointer
        this->screenCont->drawLine(0, tickHeight, 15, tickHeight, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
    }

}

void CalendarBar::drawPointer()
{
    // Draw the pointer if we are within the operating hours of the day
    if ((timeinfo->tm_hour * 60 + timeinfo->tm_min) <= (dayEndTime[0] * 60 + dayEndTime[1]))
    {
        int pointerHeight = screenMargin + int((double)((timeinfo->tm_hour * 60 + timeinfo->tm_min) - (dayStartTime[0] * 60 + dayStartTime[1])) / dayTotalMinutes * barWorkArea);
        this->screenCont->drawLine(0, pointerHeight - 5, 15, pointerHeight, BLACK, DOT_PIXEL_3X3, LINE_STYLE_SOLID);
        this->screenCont->drawLine(0, pointerHeight + 5, 15, pointerHeight, BLACK, DOT_PIXEL_3X3, LINE_STYLE_SOLID);
    }
}