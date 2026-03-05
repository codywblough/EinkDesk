# einkDesk



This project is designed to function as a device to simply sit on one's desk and display various information such as the time, date, forecast, events for the day, etc.


This project is designed for the ESP32-S3 3.97 inch e-Paper development board developed by Waveshare which can be found at the below link.

https://www.waveshare.com/esp32-s3-epaper-3.97.htm





#### Note:

Some various files used within this project were developed by Waveshare and taken from the provided example programs to enable the functionality of the development board. These files are as follows:

* src/Debug.h
* src/DEV\_Config.cpp
* src/DEV\_Config.h
* src/EPD\_3in97.cpp
* src/EPD\_3in97.h
* src/font8.cpp
* src/font12.cpp
* src/font16.cpp
* src/font20.cpp
* src/font24.cpp
* src/GUI\_Paint.cpp
* src/GUI\_Paint.h

Some modifications exist within these files but for the most part they are unmodified.

#### Functionality:
The board has a build in SD card reader so most information is stored on an SD card such as WiFi information allowing for the board to easily connect to a home network. When connected to a network, the board upon startup will determine its location and use that information for the forecast data and to have the proper time zone for the clock.
The daily event system utilizes text files containing events for the day including a title and start/end times. This information is then used to draw essentially a bar graph showing the events throughout the day in a simple manner.
