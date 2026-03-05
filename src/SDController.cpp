
#include "SDController.h"

SDController::SDController()
{
    
}

void SDController::initialize()
{
    // Initialize the SD card
    SD_MMC.setPins(SD_CLK, SD_CMD, SD_D0, SD_D1, SD_D2, SD_D3);
    if (!SD_MMC.begin("/sdcard", true)) { 
        printf("SD card failed to mount\r\n");
        return;
    }
}

void SDController::openFile(String filepath)
{
    this->openedFile = SD_MMC.open(filepath);
}

void SDController::openFile(String filepath, String mode)
{
    this->openedFile = SD_MMC.open(filepath, mode.c_str(), true);
}

void SDController::closeFile()
{
    this->openedFile.close();
}

bool SDController::exists(String filepath)
{
    return SD_MMC.exists(filepath);
}

void SDController::remove(String filepath)
{
    SD_MMC.remove(filepath);
}