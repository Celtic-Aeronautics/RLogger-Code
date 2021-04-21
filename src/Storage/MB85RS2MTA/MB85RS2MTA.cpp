#include "MB85RS2MTA.h"

#include <SPI.h>

MB85RS2MTA::MB85RS2MTA()
    : m_chipSelect(0)
    , m_spi(nullptr)
{
}

bool MB85RS2MTA::Init(uint8_t chipSelect, SPIClass* spi)
{
    m_chipSelect = chipSelect;
    m_spi = spi;

    return true;
}