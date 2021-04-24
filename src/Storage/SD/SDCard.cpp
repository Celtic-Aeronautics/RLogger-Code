#include "SDCard.h"

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

    Serial.print("Total SDCard capacity:"); Serial.println((float)capacity * 0.000512f);

    return true;
}
