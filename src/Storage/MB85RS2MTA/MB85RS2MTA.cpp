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

    digitalWrite(m_chipSelect, LOW);
    uint8_t id = m_spi->transfer((uint8_t)OPCodes::RDID);
    digitalWrite(m_chipSelect, HIGH);

    Serial.println(id);

    return true;
}