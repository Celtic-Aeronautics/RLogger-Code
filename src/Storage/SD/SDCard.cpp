#include "SDCard.h"

#include "Utils/Debug/DebugOutput.h"

#include <SdFat.h>

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
    return true;
}
