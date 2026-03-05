
#ifndef SCREEN_CONTROLLER_H
#define SCREEN_CONTROLLER_H

#include <Arduino.h>
#include "waveshare/EPD_3in97.h"
#include "waveshare/GUI_Paint.h"
#include "waveshare/fonts.h"

#include "SDController.h"

class ScreenController
{
    public:
    // Create a new image cache
    UBYTE *BlackImage;
    UWORD Imagesize = ((EPD_3IN97_WIDTH % 8 == 0)? (EPD_3IN97_WIDTH / 8 ): (EPD_3IN97_WIDTH / 8 + 1)) * EPD_3IN97_HEIGHT;
    SDController *sdCont;
    char bmpBuffer[16];
    uint16_t twoByteTemp;
    uint32_t fourByteTemp;
    uint32_t fileSize;
    uint32_t pixelDataOffset;
    uint32_t imgWidth;
    int32_t imgHeight;
    

    ScreenController();
    void initialize();

    int getDisplayWidth();
    int getDisplayHeight();

    void createBlankImage();
    void createPartialBlankImage(uint16_t width, uint16_t height);
    void display();
    void displayPartial(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd);
    void drawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color, DOT_PIXEL lineWidth, LINE_STYLE lineStyle);
    void drawRectangle(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color, DOT_PIXEL lineWidth, DRAW_FILL drawFill);
    void writeString(uint16_t xStart, uint16_t yStart, const char *text, sFONT *font, uint16_t foregroundColor, uint16_t backgroundColor, uint8_t scale, uint8_t letterSpacing);
    void drawBitmap(String filepath, uint16_t xStart, uint16_t yStart);
};
#endif