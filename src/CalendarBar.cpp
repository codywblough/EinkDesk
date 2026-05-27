
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
        title.trim();
        line = this->sdCont->openedFile.readStringUntil('\n');
        line.trim();
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

void CalendarBar::drawEventInfo()
{
    // Get minutes since midnight for right now
    int currentMins = timeinfo->tm_hour * 60 + timeinfo->tm_min;
    // Check if there are any events today
    if (this->eventCount > 0)
    {
        // If so, loop through them to see if currentMins falls between a start and end time for an event
        int currEventIdx = -1; // Default -1 so we can easily tell if we aren't in an event
        int nextEventIdx = -1; // Default -1 so we can easily tell if there is nothing after
        for (int i = 0; i < this->eventCount; i++)
        {
            // Save the index of the next event that ended (and check that this hasn't been set yet so we don't keep reassigning)
            if (currentMins < this->calendarEvents[i].getStartTimeMins() && nextEventIdx == -1)
                nextEventIdx = i;
            if (currentMins > this->calendarEvents[i].getStartTimeMins() && currentMins < this->calendarEvents[i].getEndTimeMins())
            {
                // Save the current event index
                currEventIdx = i;
                
                // Check that we aren't on the last event. If we aren't, save the next event's index
                if (i < this->eventCount - 1)
                    nextEventIdx = i + 1;

                // Don't neet to check any other events
                break;
            }
            
            
        }

        int maxSmallChars = 20;
        int maxBigChars = 13;
        String tempEvent = "";

        // Event printing handling:
        // Current Event:
        if (currEventIdx == -1)
        {
            // No event in progress
            this->screenCont->writeString(90, 70, "No Event", &Font16, BLACK, WHITE, 2, 0);
        }
        else
        {
            // Display the current event
            tempEvent = this->calendarEvents[currEventIdx].getTitle().c_str();
            if (tempEvent.length() > maxBigChars)
                tempEvent = tempEvent.substring(0, maxBigChars - 3) + "...";
            this->screenCont->writeString(90, 70, (const char*)(tempEvent.c_str()), &Font16, BLACK, WHITE, 2, 0);
            // Tell user when it ends
            char buffer[16];
            int tempMins = this->calendarEvents[currEventIdx].getEndTimeMins();
            int tempHour = tempMins / 60;
            if (tempHour > 12)
                tempHour -= 12;
            sprintf(buffer, "Ends at %d:%d%s", tempHour, tempMins % 60, tempHour > 11 ? "am" : "pm");
            this->screenCont->writeString(90, 110, (const char*)(buffer), &Font24, BLACK, WHITE, 1, 1);
        }
        
        // Next event:
        // Draw a divider line
        this->screenCont->drawLine(80, 150, 370, 150, BLACK, DOT_PIXEL_2X2, LINE_STYLE_SOLID);
        if (nextEventIdx == -1)
        {
            // No upcoming events
            this->screenCont->writeString(80, 155, "No Event", &Font12, BLACK, WHITE, 2, 0);
        }
        else
        {
            // Display the upcoming event
            tempEvent = this->calendarEvents[nextEventIdx].getTitle().c_str();
            if (tempEvent.length() > maxSmallChars)
                tempEvent = tempEvent.substring(0, maxSmallChars - 3) + "...";
            this->screenCont->writeString(80, 155, (const char*)(tempEvent.c_str()), &Font12, BLACK, WHITE, 2, 0);
            // Tell user when it starts
            char buffer[18];
            int tempMins = this->calendarEvents[nextEventIdx].getStartTimeMins();
            int tempHour = tempMins / 60;
            if (tempHour > 12)
                tempHour -= 12;
            sprintf(buffer, "Starts at %d:%d%s", tempHour, tempMins % 60, tempHour > 11 ? "am" : "pm");
            this->screenCont->writeString(80, 185, (const char*)(buffer), &Font12, BLACK, WHITE, 2, 0);
        }
    }
    else
    {
        // If there are no events today, just display "No Event"
        this->screenCont->writeString(90, 70, "Schedule Clear", &Font16, BLACK, WHITE, 2, 0);
    }
    
}

void CalendarBar::drawCalendar()
{
    int month = timeinfo->tm_mon + 1;
    int year = timeinfo->tm_year + 1900;

    // 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat
    int firstDay = findStartingDay(year, month, 1);
    int dayCount = daysInMonth[month - 1];
    if (month == 2)
        dayCount += isLeapYear(year);

    // Each square of calendar is 30x30 pixels with 5 pixel spacing

    int xStart = 105;
    int yStart = 225;

    // Print the day labels (S,M,T,W,T,F,S)
    this->screenCont->writeString(xStart,        yStart, "S", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*1+5, yStart, "M", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*2+5, yStart, "T", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*3+5, yStart, "W", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*4+5, yStart, "T", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*5+5, yStart, "F", &Font16, BLACK, WHITE, 2, 0);
    this->screenCont->writeString(xStart+35*6+5, yStart, "S", &Font16, BLACK, WHITE, 2, 0);
    yStart += 35;
    // Draw squares for each day of the month
    int curCol = firstDay;
    int curRow = 0;
    int tempX = 0;
    int tempY = 0;
    for (int i = 0; i < dayCount; i++)
    {
        // Move to the next row and reset the column
        if ((i + firstDay) % 7 == 0 && i != 0)
        {
            curRow++;
            curCol = 0;
        }
        tempX = xStart + curCol * 35;
        tempY = yStart + curRow * 35;
        if (timeinfo->tm_mday == i + 1)
            this->screenCont->drawRectangle(tempX, tempY, tempX + 30, tempY + 30, BLACK, DOT_PIXEL_2X2, DRAW_FILL_FULL);
        else
            this->screenCont->drawRectangle(tempX, tempY, tempX + 30, tempY + 30, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
        curCol++;
    }
}

int CalendarBar::isLeapYear(int year)
{
    return ((year % 4 == 0) && ((year % 400 == 0) || (year % 100 != 0)));
}

int CalendarBar::findStartingDay(int year, int month, int day)
{
    // Find year code
    int temp = year % 100;
    temp = temp * 1.25;
    // Add month code
    switch (month)
    {
        case 4: // Apr
        case 7: // Jul
            // += 0
            break;
        case 1: // Jan
        case 10: // Oct
            temp += 1;
            break;
        case 5: // May
            temp += 2;
            break;
        case 8: // Aug
            temp += 3;
            break;
        case 2: // Feb
        case 3: // Mar
        case 11: // Nov
            temp += 4;
            break;
        case 6: // Jun
            temp += 5;
            break;
        case 9: // Sep
        case 12: // Dec
            temp += 6;
            break;
        default: // This should NEVER be reached
            return -1;
    }
    // Add century code
    if (year >= 2000)
    {
        temp += 6;
    }
    else
    {
        return -1; // This really shouldn't happen either until 2100 or for some reason before the 2000s
    }
    // Add day number
    temp += day;
    // Adjust for leap years
    if (month < 3)
        temp -= isLeapYear(year); // If it is a leap year it will subtract 1
    // Modulo by 7
    temp = temp % 7;
    // 1=Sun, 2=Mon, 3=Tue, 4=Wed, 5=Thu, 6=Fri, 0=Sat
    // Adjust that number a little
    temp -= 1;
    if (temp < 0)
        temp = 6;
    // 0=Sun, 1=Mon, 2=Tue, 3=Wed, 4=Thu, 5=Fri, 6=Sat
    return temp;
}