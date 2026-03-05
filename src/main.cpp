#include <Arduino.h>
#include "waveshare/EPD_3in97.h"
#include "waveshare/GUI_Paint.h"
#include "waveshare/fonts.h"
// #include "SD_MMC.h"
// #include "FS.h"
// #include "esp_vfs_fat.h"
// #include "sdmmc_cmd.h"
// #include "driver/sdmmc_host.h"

#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "ScreenController.h"
#include "SDController.h"
#include "CalendarBar.h"
#include "Forecast.h"

// WiFi variables
String ssid = "";
String password = "";
long wifiTimeout = 10 * 1000; // Stop attempting to connect to WiFi after 10s

const char* ntpServer = "pool.ntp.org";
long gmtOffset_sec = 0;

struct tm timeinfo;
String jsonBuffer;
JSONVar jsonObject;

int lastDay = -1; // Used for tracking if the day has changed (Daily updates)
int lastMin = -1; // Used for tracking if the minute has changed (Minutely updates)

long lastMillis = 0;

// Class object initializations
ScreenController screenCont = ScreenController();
SDController sdCont = SDController();
CalendarBar calBar = CalendarBar();
Forecast forecast = Forecast();

// Prototype Function Statements
void update();
void calibrateTime();
void displayTimeInfo();
String httpGetRequest();


void setup()
{
    // Initialize the board
    DEV_Module_Init();
    Serial.begin(115200);
    Serial.println("\nProgram Starting...");
    Serial.println("GO!");

    // Initialize the screen controller
    screenCont.initialize();
    // Initialize the SD Controller
    sdCont.initialize();

    // Connect to WiFi
    sdCont.openFile("/wifi.txt");
    if (sdCont.openedFile)
    {
        // If able to open the wifi file, read it
        ssid = sdCont.openedFile.readStringUntil('\n');
        password = sdCont.openedFile.readStringUntil('\n');
        ssid.trim(); // Remove the pesky \r at the end
        password.trim(); // Remove the pesky \r at the end
        Serial.println((const char*)ssid.c_str());
        Serial.println((const char*)password.c_str());
        WiFi.begin((const char*)ssid.c_str(), (const char*)password.c_str());
        lastMillis = millis();
        while (WiFi.status() != WL_CONNECTED && (millis() - lastMillis) < wifiTimeout)
        {
            delay(100);
            Serial.print(".");
            // Should probably include timeout functionality and displaying on the screen instead of
            // infinite looping and only printing to the serial monitor
        }
    }
   
    // Check if we were able to connect to the network
    if (WiFi.status() == WL_CONNECTED)
    {
        // Successfully connected to the network
        Serial.println("\nWiFi Connected!");
        // Make a IP request to get geolocation data
        jsonBuffer = httpGetRequest();
        jsonObject = JSON.parse(jsonBuffer);
        // Process the data
        // TODO: Add a check to not overwrite data if failed to make the IP API call
        // Get the time offset for time zone
        gmtOffset_sec = (long)jsonObject["offset"];
        // Get the zip code
        forecast.zipCode = (String)(const char*)(jsonObject["zip"]);
        forecast.zipCode.trim();
        // Get the country code
        forecast.countryCode = (String)(const char*)jsonObject["countryCode"];
        forecast.countryCode.trim();
        forecast.locationDataRead = true;
        
        // Save found data to file for use if unable to connect to the internet later
        if (sdCont.exists("/previousGeoData.txt"))
            sdCont.remove("/previousGeoData.txt");
        if (sdCont.openedFile)
        {
            sdCont.openFile("/previousGeoData.txt", "w");
            // Save the time zone offset
            sdCont.openedFile.println((String)gmtOffset_sec);
            // Save the zip code
            sdCont.openedFile.println(forecast.zipCode);
            // Save the country code
            sdCont.openedFile.println(forecast.countryCode);
            sdCont.closeFile();
        }
    }
    else
    {
        // Unable to connect to network or failed to open wifi file
        Serial.println("Failed to retrieve WiFi information");
        // Pull geolocation data from data saved from last successful connection
        sdCont.openFile("/previousGeoData.txt");
        if (sdCont.openedFile)
        {
            // Format in file is:
            // Time Zone Offset
            // Zip Code
            // Country Code
            gmtOffset_sec = sdCont.openedFile.readStringUntil('\n').toInt();
            forecast.zipCode = sdCont.openedFile.readStringUntil('\n');
            forecast.zipCode.trim();
            forecast.countryCode = sdCont.openedFile.readStringUntil('\n');
            forecast.countryCode.trim();
        }
    }

    // Get the current time from the ntp server
    configTime(gmtOffset_sec, 0, ntpServer);  

    // Configure the various classes
    calBar.screenCont = &screenCont; // Give CalendarBar reference to ScreenController
    forecast.screenCont = &screenCont; // Give Forecast reference to ScreenController
    screenCont.sdCont = &sdCont; // Give ScreenController reference to SDController
    calBar.sdCont = &sdCont; // Give CalendarBar reference to SDController
    forecast.sdCont = &sdCont; // Give Forecast reference to SDController
    calBar.timeinfo = &timeinfo; // Give CalendarBar reference to timeinfo

    // Initialize the CalendarBar
    calBar.initialize();
    // Initialize classes that require WiFi
    forecast.initialize();

    calibrateTime(); // Calibrate the device time
    lastMillis = millis(); // Save current millis
    
}

void loop()
{
    // put your main code here, to run repeatedly:
    // Do updates every 0.5s (essentially doing this to slow down loop)
    if (millis() - lastMillis > 500)
    {
        lastMillis = millis();
        calibrateTime();

        update();
    }
}

void update()
{
    // Daily updates
    if (timeinfo.tm_mday != lastDay)
    {
        Serial.println("Daily Update");
        // Save the new day
        lastDay = timeinfo.tm_mday;
        // Do a full screen refresh
        screenCont.createBlankImage();

        // Refresh the calendar bar events
        calBar.initCalendarBarEvents();
        // Draw the new calendar bar
        calBar.drawCalendarBar();
        calBar.drawPointer();

        // Display the image buffer
        screenCont.display();
        DEV_Delay_ms(3000);
    }
    // Update display every minute
    if (timeinfo.tm_min != lastMin)
    {
        Serial.println("Minutely Update");
        // Save the new minute
        lastMin = timeinfo.tm_min;
        
        // Create a blank image
        screenCont.createBlankImage();

        // Draw the calendar bar and the pointer
        calBar.drawCalendarBar();
        calBar.drawPointer();
        // Draw the clock/date
        displayTimeInfo();
        // Forecast update
        forecast.updateForecast();

        // Trying to figure out partial refresh here
        // screenCont.createPartialBlankImage(400, 480);
        // displayTimeInfo();
        // screenCont.displayPartial(400, 0, 800, 480);

        // Display the image buffer
        screenCont.display();
    }
}

void calibrateTime()
{
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("Failed to obtain time");
    }
}

void displayTimeInfo()
{
    char timeHour[6];
    char timePeriod[3];
    char timeDayName[10];
    char timeDate[13];
    strftime(timeHour, 6, "%I:%M", &timeinfo);
    strftime(timePeriod, 3, "%P", &timeinfo);
    strftime(timeDayName, 10, "%A", &timeinfo);
    strftime(timeDate, 13, "%b %d, %Y", &timeinfo);

    // screenCont.drawRectangle(10, 10, 50, 50, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
    // screenCont.writeString(0, 60, timeHour, &Font24, BLACK, WHITE, 4, 4);
    // screenCont.writeString(270, 84, timePeriod, &Font24, BLACK, WHITE, 3, 2);
    // screenCont.writeString(0, 170, timeDayName, &Font24, BLACK, WHITE, 2, 2);
    // screenCont.writeString(0, 220, timeDate, &Font24, BLACK, WHITE, 2, 2);
    screenCont.writeString(400, 60, timeHour, &Font24, BLACK, WHITE, 4, 4);
    screenCont.writeString(670, 84, timePeriod, &Font24, BLACK, WHITE, 3, 2);
    screenCont.writeString(400, 170, timeDayName, &Font24, BLACK, WHITE, 2, 2);
    screenCont.writeString(400, 220, timeDate, &Font24, BLACK, WHITE, 2, 2);
}

// Perform API request
String httpGetRequest()
{
    HTTPClient http;
    http.begin("http://ip-api.com/json?fields=status,message,countryCode,zip,offset");
    int httpCode = http.GET();

    String payload = "{}";

    if (httpCode == 200) {
        payload = http.getString();
        Serial.println(payload);
    }
    http.end();

    return payload;
}