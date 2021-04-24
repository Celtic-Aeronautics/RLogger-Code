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

    //SPISettings settings(1000000, MSBFIRST, SPI_MODE0);
    SPISettings settings;

    m_spi->beginTransaction(settings);
    digitalWrite(m_chipSelect, LOW);
    m_spi->transfer((uint8_t)OPCodes::RDID);
    
    delay(10);

    uint8_t data[4] = {0xff, 0x8, 0x8, 0x8};
    data[0] = m_spi->transfer(data[0]);
    m_spi->transfer(&data[1], 1);
    m_spi->transfer(&data[2], 1);
    m_spi->transfer(&data[3], 1);

    digitalWrite(m_chipSelect, HIGH);
    m_spi->endTransaction();

    Serial.println(data[0]);
    Serial.println(data[1]);
    Serial.println(data[2]);
    Serial.println(data[3]);

    return true;
}