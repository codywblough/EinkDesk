
#include "Forecast.h"

Forecast::Forecast()
{

}

void Forecast::initialize()
{
    // Get the API related data from the txt file
    this->sdCont->openFile("/forecast/location.txt");
    if (!this->sdCont->openedFile)
    {
        Serial.println("Failed to open forecast file");
        return;
    }

    // Format in file is:
    // API Key
    // City Code
    // State Code
    // Country Code
    apiKey = this->sdCont->openedFile.readStringUntil('\n');
    apiKey.trim();
    //zipCode = this->sdCont->openedFile.readStringUntil('\n');
    //zipCode.trim();
    //countryCode = this->sdCont->openedFile.readStringUntil('\n');
    //countryCode.trim();
    //locationDataRead = true;
    this->sdCont->closeFile();
}

// Get updated data and put it on the display
void Forecast::updateForecast()
{
    if (!locationDataRead)
    {
        Serial.println("Failed to update forecast as failed to read location data earlier");
        return;
    }

    // First check if we are connected to the internet
    if (WiFi.status() == WL_CONNECTED)
    {
        jsonBuffer = httpGetRequest();
        jsonObject = JSON.parse(jsonBuffer);

        // Pull out the important information
        temperature = (int)round((double)jsonObject["main"]["feels_like"]); // Grab 'feels like' for temperature
        conditionId = jsonObject["weather"][0]["id"];
        evaluateCondition();
        dataAvailable = true;        

    }

    // Display on the screen
    // Draw the box
    screenCont->drawRectangle(400, 280, 550, 460, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY);
    // Write the temperature
    if (dataAvailable == false) // No data available so print "--"
        screenCont->writeString(437, 400, "--", &Font24, BLACK, WHITE, 2, 2);
    else if (temperature >= 100 || temperature <= -10) // 3 digits
    {
        screenCont->writeString(405, 400, ((String)temperature).c_str(), &Font24, BLACK, WHITE, 2, 2);
        screenCont->writeString(495, 390, "o", &Font24, BLACK, WHITE, 1, 2); // 'Degree' symbol
    }
    else // 2 digits
    {
        screenCont->writeString(437, 400, ((String)temperature).c_str(), &Font24, BLACK, WHITE, 2, 2);
        screenCont->writeString(495, 390, "o", &Font24, BLACK, WHITE, 1, 2); // 'Degree' symbol
    }
        
    // Draw the correct image (80x80px)
    if (dataAvailable)
        screenCont->drawBitmap(conditionPath, 435, 290);
    else
        screenCont->drawRectangle(435, 290, 515, 370, BLACK, DOT_PIXEL_2X2, DRAW_FILL_EMPTY); // Placeholder to get vibe
}

void Forecast::evaluateCondition()
{
    switch (conditionId)
    {
        // Clear Sky Icon (01)
        case 800:
            Serial.println("Clear Icon");
            conditionPath = "/forecast/01.bmp";
            break;
        
        // Few Clouds Icon (02)
        case 801:
            Serial.println("Few Clouds Icon");
            conditionPath = "/forecast/02.bmp";
            break;

        // Scattered Clouds Icon (03)
        case 802:
            Serial.println("Scattered Clouds Icon");
            conditionPath = "/forecast/03.bmp";
            break;

        // Broken Clouds Icon (04)
        case 803:
        case 804:
            Serial.println("Broken Clouds Icon");
            conditionPath = "/forecast/04.bmp";
            break;

        // Shower Rain Icon (09)
        case 300:
        case 301:
        case 302:
        case 310:
        case 311:
        case 312:
        case 313:
        case 314:
        case 321:
        case 520:
        case 521:
        case 522:
        case 531:
            Serial.println("Shower Rain Icon");
            conditionPath = "/forecast/09.bmp";
            break;

        // Rain Icon (10)
        case 500:
        case 501:
        case 502:
        case 503:
        case 504:
            Serial.println("Rain Icon");
            conditionPath = "/forecast/10.bmp";
            break;

        // Thunderstorm Icon (11)
        case 200:
        case 201:
        case 202:
        case 210:
        case 211:
        case 212:
        case 221:
        case 230:
        case 231:
        case 232:
            Serial.println("Thunderstorm Icon");
            conditionPath = "/forecast/11.bmp";
            break;

        // Snow Icon (13)
        case 600:
        case 601:
        case 602:
        case 611:
        case 612:
        case 613:
        case 615:
        case 616:
        case 620:
        case 621:
        case 622:
        case 511:
            Serial.println("Snow Icon");
            conditionPath = "/forecast/13.bmp";
            break;

        // Mist Icon (50)
        case 701:
        case 711:
        case 721:
        case 731:
        case 741:
        case 751:
        case 761:
        case 762:
        case 771:
        case 781:
            Serial.println("Mist Icon");
            conditionPath = "/forecast/50.bmp";
            break;
        default:
            Serial.println("Default");
            conditionPath = "/forecast/01.bmp"; // Default to clear sky
            break;
    }
}

// Perform API request
String Forecast::httpGetRequest()
{
    //WiFiClient client;
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    const char* host = "api.openweathermap.org";
    String path = "/data/2.5/weather?zip=" + zipCode + "," + countryCode + "&units=imperial&APPID=" + apiKey;

    http.begin(client, host, 443, (const char*)path.c_str(), true);
    int httpResponseCode = http.GET();

    String payload = "{}";

    if (httpResponseCode > 0)
        payload = http.getString();
    else
    {
        // HTTP Error code
    }
    http.end();

    return payload;
}