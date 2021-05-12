#include "SDCard.h"

#include "Utils/Debug/DebugOutput.h"

#include <SdFat.h>

#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI)

SDCard::SDCard()
    : m_chipSelect(0)
{
}

bool SDCard::Init(uint8_t chipSelect)
{
    m_chipSelect = chipSelect;

    SdFat sd;
    if(!sd.begin(chipSelect))
    {
        return false;
    }
    
    csd_t csd = {};
    sd.card()->readCSD(&csd);
    uint32_t capacity = sdCardCapacity(&csd);

    DEBUG_LOG("Total SDCard capacity:%f MB", (float)capacity * 0.000512f);

    delay(50);

    return true;
}

void SDCard::TestWrite()
{
    

    File f;
    if(f.open("log1.csv", O_CREAT))
    {
        f.close();
    }
    else
    {
        DEBUG_LOG("Failed to run the SD card Test %i", f.getError());
    }
}