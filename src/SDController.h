
#ifndef SD_CONTROLLER_H
#define SD_CONTROLLER_H

#include <Arduino.h>
#include "SD_MMC.h"
#include "FS.h"
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#define SD_CLK  16
#define SD_CMD  17
#define SD_D0   15
#define SD_D1   7
#define SD_D2   8
#define SD_D3   18

#define MOUNT_POINT "/sdcard"

class SDController
{
    public:
    File openedFile;
    SDController();
    void initialize();
    void openFile(String filepath);
    void openFile(String filepath, String mode);
    void closeFile();
    bool exists(String filepath);
    void remove(String filepath);
};

#endif