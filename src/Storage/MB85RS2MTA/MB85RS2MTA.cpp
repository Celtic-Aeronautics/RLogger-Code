#include "MB85RS2MTA.h"

#include <SPI.h>

#define MB_EXTRA_CHECKS 0

MB85RS2MTA::MB85RS2MTA()
    : m_chipSelect(0)
    , m_spi(nullptr)
    , m_spiSettings(nullptr)
{
}

bool MB85RS2MTA::Init(uint8_t chipSelect, SPIClass* spi)
{
    m_chipSelect = chipSelect;
    m_spi = spi;

    m_spiSettings = new SPISettings;

    // Read the device ID
#if MB_EXTRA_CHECKS == 1
    BeginTransaction();
    {
        uint8_t data[4] = {0xff, 0x8, 0x8, 0x8};
        m_spi->transfer((uint8_t)OPCodes::RDID);
        data[0] = m_spi->transfer(data[0]);
        m_spi->transfer(&data[1], 1);
        m_spi->transfer(&data[2], 1);
        m_spi->transfer(&data[3], 1);
    }
    EndTransaction();
#endif

    // Test addr code
#if MB_EXTRA_CHECKS == 1
    uint8_t tempValues[3];
    uint32_t tempAddr = 75338;
    SplitAddress(tempAddr, tempValues);
    if(CombineAddress(tempValues) != tempAddr)
    {
        Serial.println("Address logic not working");
    }
#endif

    // Enable writes
    WriteCommand(OPCodes::WREN);

    return true;
}

void MB85RS2MTA::Write(const uint32_t address, const uint8_t value)
{
    Write(address, (void*)&value, 1);
}

void MB85RS2MTA::Write(const uint32_t address, void* data, uint8_t dataSize)
{
    uint8_t addrBits[3];
    SplitAddress(address, addrBits);

    BeginTransaction();
    {
        m_spi->transfer((uint8_t)OPCodes::WRITE);
        m_spi->transfer(addrBits[0]);
        m_spi->transfer(addrBits[1]);
        m_spi->transfer(addrBits[2]);
        for(uint8_t cur = 0; cur < dataSize; ++cur)
        {
            m_spi->transfer(((uint8_t*)data)[cur]);
        }
    }
    EndTransaction();
}

uint8_t MB85RS2MTA::Read(const uint32_t address)
{
    uint8_t readValue = 0xff;
    uint8_t addrBits[3];
    SplitAddress(address, addrBits);

    BeginTransaction();
    {
        m_spi->transfer((uint8_t)OPCodes::READ);
        m_spi->transfer(addrBits[0]);
        m_spi->transfer(addrBits[1]);
        m_spi->transfer(addrBits[2]);
        m_spi->transfer(&readValue,1);
    }
    EndTransaction();

    return readValue;
}

void MB85RS2MTA::Read(const uint32_t address, void* data, uint8_t dataSize)
{
    // need to think about this one
    /*
    uint8_t readValue = 0xff;
    uint8_t addrBits[3];
    SplitAddress(address, addrBits);

    BeginTransaction();
    {
        m_spi->transfer((uint8_t)OPCodes::READ);
        m_spi->transfer(addrBits[0]);
        m_spi->transfer(addrBits[1]);
        m_spi->transfer(addrBits[2]);
        for(uint8_t cur = 0; cur < dataSize; ++cur)
        {
            m_spi->transfer(&readValue, 1);
            memcpy(&readValue, data + cur, 1);
        }
    }
    EndTransaction();
    */
}

void MB85RS2MTA::SplitAddress(const uint32_t address, uint8_t* values)
{
    values[0] = (address >> 16u) & 0x3u;
    values[1] = (uint8_t)(address >> 8u);
    values[2] = (uint8_t)(address);
}

uint32_t MB85RS2MTA::CombineAddress(uint8_t* values)
{
    return (uint32_t)values[0] << 16u | (uint32_t)values[1] << 8u | (uint32_t)values[2];
}

void MB85RS2MTA::BeginTransaction()
{
    m_spi->beginTransaction(*m_spiSettings);
    digitalWrite(m_chipSelect, LOW);
}

void MB85RS2MTA::EndTransaction()
{
    digitalWrite(m_chipSelect, HIGH);
    m_spi->endTransaction();
}

void MB85RS2MTA::WriteCommand(const OPCodes command)
{
    BeginTransaction();
    m_spi->transfer((uint8_t)command);
    EndTransaction();
}