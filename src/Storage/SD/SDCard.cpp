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

    return true;
}
