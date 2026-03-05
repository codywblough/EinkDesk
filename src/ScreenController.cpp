
#include "ScreenController.h"

ScreenController::ScreenController()
{

}

void ScreenController::initialize()
{
    // Initialize the screen
    EPD_3IN97_Init();
    // Clear the screen
    EPD_3IN97_Clear();
    DEV_Delay_ms(2000);
    // Allocate screen buffer
    if((BlackImage = (UBYTE *)malloc(Imagesize)) == NULL) {
        printf("Failed to apply for black memory...\r\n");
        while(1);
    }
    DEV_Delay_ms(2000);
}

int ScreenController::getDisplayWidth()
{
    return EPD_3IN97_WIDTH;
}

int ScreenController::getDisplayHeight()
{
    return EPD_3IN97_HEIGHT;
}

void ScreenController::createBlankImage()
{
    // Create a blank image to draw onto
    EPD_3IN97_Init();
    Paint_NewImage(BlackImage, EPD_3IN97_WIDTH, EPD_3IN97_HEIGHT, ROTATE_180, WHITE);
    Paint_SetScale(2);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
}

void ScreenController::createPartialBlankImage(uint16_t width, uint16_t height)
{
    Paint_NewImage(BlackImage, width, height, ROTATE_180, WHITE);
    Paint_SelectImage(BlackImage);
    Paint_Clear(WHITE);
    Paint_ClearWindows(0, 0, width, height, WHITE);
}

void ScreenController::display()
{
    EPD_3IN97_Display_Base(BlackImage);
}

void ScreenController::displayPartial(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd)
{
    EPD_3IN97_Display_Partial(BlackImage, xStart, yStart, xEnd, yEnd);
}

void ScreenController::drawLine(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color, DOT_PIXEL lineWidth, LINE_STYLE lineStyle)
{
    Paint_DrawLine(xStart, yStart, xEnd, yEnd, color, lineWidth, lineStyle);
}

void ScreenController::drawRectangle(uint16_t xStart, uint16_t yStart, uint16_t xEnd, uint16_t yEnd, uint16_t color, DOT_PIXEL lineWidth, DRAW_FILL drawFill)
{
    Paint_DrawRectangle(xStart, yStart, xEnd, yEnd, color, lineWidth, drawFill);
}

void ScreenController::writeString(uint16_t xStart, uint16_t yStart, const char *text, sFONT *font, uint16_t foregroundColor, uint16_t backgroundColor, uint8_t scale, uint8_t letterSpacing)
{
    Paint_DrawString_EN_Scaled(xStart, yStart, text, font, foregroundColor, backgroundColor, scale, letterSpacing);
}

void ScreenController::drawBitmap(String filepath, uint16_t xStart, uint16_t yStart)
{
    Serial.println("\nBeginning of drawBitmap()");
    this->sdCont->openFile(filepath);
    // Bitmaps are little endian so any multi-byte value needs to be reversed
    // Additionally bitmaps are usually ordered as bottom->top. Positive height represents bottom->top, negative height represents top->bottom
    // Bitmap rows for image data must be in multiples of 4 bytes, so padding may be added if this isn't the case

    // Bitmap Header (14 bytes)
    // offset | size | Meaning
    // 0      | 2    | Signature "BM"
    // 2      | 4    | File size
    // 10     | 4    | Offset to pixel data
    // First 2 bytes are "BM" signature
    this->sdCont->openedFile.readBytes(bmpBuffer, 2);
    Serial.print(bmpBuffer[0]);
    Serial.println(bmpBuffer[1]);
    // Move to the File size field
    this->sdCont->openedFile.seek(2);
    this->sdCont->openedFile.readBytes(bmpBuffer, 4);
    fileSize = bmpBuffer[0] + (bmpBuffer[1] << 8) + (bmpBuffer[2] << 16) + (bmpBuffer[3] << 24); // Handle little-endianess
    Serial.println(fileSize);
    // Move to pixel data offset
    this->sdCont->openedFile.seek(10);
    this->sdCont->openedFile.readBytes(bmpBuffer, 4);
    pixelDataOffset = bmpBuffer[0] + (bmpBuffer[1] << 8) + (bmpBuffer[2] << 16) + (bmpBuffer[3] << 24); // Handle little-endianess
    Serial.println(pixelDataOffset);

    // Bitmap Info Header (40 bytes)
    // offset | size | Meaning
    // 14     | 4    | Header size (40)
    // 18     | 4    | Width (pixels)
    // 22     | 4    | Height (pixels)
    // 26     | 2    | Planes (always 1)
    // 28     | 2    | Bits per pixel (1 for 1-bit BMP)
    // 30     | 4    | Compression (0=None)
    // 34     | 4    | Image data size (May be 0 for uncompressed)
    // Header size
    this->sdCont->openedFile.seek(14);
    this->sdCont->openedFile.readBytes(bmpBuffer, 4);
    fourByteTemp = bmpBuffer[0] + (bmpBuffer[1] << 8) + (bmpBuffer[2] << 16) + (bmpBuffer[3] << 24); // Handle little-endianess
    Serial.println(fourByteTemp);
    // Width
    this->sdCont->openedFile.seek(18);
    this->sdCont->openedFile.readBytes(bmpBuffer, 4);
    fourByteTemp = bmpBuffer[0] + (bmpBuffer[1] << 8) + (bmpBuffer[2] << 16) + (bmpBuffer[3] << 24); // Handle little-endianess
    imgWidth = fourByteTemp;
    Serial.println(imgWidth);
    // Height
    this->sdCont->openedFile.seek(22);
    this->sdCont->openedFile.readBytes(bmpBuffer, 4);
    fourByteTemp = bmpBuffer[0] + (bmpBuffer[1] << 8) + (bmpBuffer[2] << 16) + (bmpBuffer[3] << 24); // Handle little-endianess
    imgHeight = (int32_t)fourByteTemp; // Height is signed due to possibility of being negative (top->bottom formatting)
    Serial.println(imgHeight);

    // Pixel Data
    this->sdCont->openedFile.seek(pixelDataOffset); // Move to the offset where the data starts
    int widthCounter = 0;
    int heightCounter = 0;
    int padBytes = 4 - (((imgWidth + 7) / 8) % 4);
    bool topBottom = false; // Used for identifying if file is top->bottom or bottom->top
    if (imgHeight < 0)
    {
        topBottom = true; // Set flag for top->bottom configuration
        imgHeight = imgHeight * -1; // Just make it positive for now
        heightCounter = 0;
    }
    else
        heightCounter = imgHeight - 1; // If bottom->top, we need to start from height and decrement
   
    // Put our image on the display
    // NOTE: No handling in place for an image that isn't divisible into bytes
    while(this->sdCont->openedFile.available() != 0)
    {
        // Read a byte
        this->sdCont->openedFile.readBytes(bmpBuffer, 1);
        // Loop through each bit of the byte to draw the respective pixels
        for (int bit = 7; bit >= 0; bit--)
        {
            // Check if we should paint a pixel black or white
            if ((bmpBuffer[0] >> bit) & 1)
                Paint_SetPixel(xStart + widthCounter + 7 - bit, yStart + heightCounter, WHITE);
            else
                Paint_SetPixel(xStart + widthCounter + 7 - bit, yStart + heightCounter, BLACK);
        }
        widthCounter += 8; // Increment width counter by 1 byte

        // If we've reached the end of a row, increment the height counter
        if (widthCounter >= imgWidth)
        {
            // Go to next row
            if (topBottom)
                heightCounter++; // Increment row if top->bottom config
            else
                heightCounter--; // Decrement row if bottom->top config
            // Go back to the start of the column
            widthCounter = 0;
            // Read in the padding bytes to get rid of them
            this->sdCont->openedFile.readBytes(bmpBuffer, padBytes);
        }
    }

    this->sdCont->closeFile();
    Serial.println("End of drawBitmap()\n");
}