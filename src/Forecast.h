
#ifndef _FORECAST_H
#define _FORECAST_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>

#include "ScreenController.h"
#include "SDController.h"

class Forecast
{
    public:
    ScreenController *screenCont;
    SDController *sdCont;

    // /forecast/location.txt contains this info
    String apiKey = "";
    String zipCode = "";
    String countryCode = "";
    String jsonBuffer;
    JSONVar jsonObject;

    bool locationDataRead = false;
    bool dataAvailable = false;
    int temperature;
    int conditionId;
    String conditionPath = "";

    Forecast();
    void initialize();
    void updateForecast();
    void evaluateCondition();
    String httpGetRequest();

};


#endif