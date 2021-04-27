#pragma once

#include <stdint.h>

class SPIClass;
class SPISettings;

// Memory FRAM
// Datasheet: https://cdn-shop.adafruit.com/product-files/4718/4718_MB85RS2MTA.pdf
class MB85RS2MTA
{
public:
    MB85RS2MTA();

    bool Init(uint8_t chipSelect, SPIClass* spi);

    // Writes a single value to address
    void Write(const uint32_t address, const uint8_t value);

    // Writes a block of data starting at address
    void Write(const uint32_t address, uint8_t* data, uint8_t dataSize);

    // Reads single value from address
    uint8_t Read(const uint32_t address);

    // Reads a block of data starting at address
    void Read(const uint32_t address, uint8_t* data, uint8_t dataSize);

private:
    enum class OPCodes : uint8_t
    {
        WREN   = 0x6,
        WRDI   = 0x4,
        RDSR   = 0x5,
        WRSR   = 0x1,
        READ   = 0x3,
        WRITE  = 0x2,
        RDID   = 0x9F,
        FSTRD  = 0xB,
        SLEEP  = 0xB9,
    };

    void SplitAddress(const uint32_t address, uint8_t* values);
    
    uint32_t CombineAddress(uint8_t* values);

    void BeginTransaction();

    void EndTransaction();

    void WriteCommand(const OPCodes command);

    uint8_t m_chipSelect;
    SPIClass* m_spi;
    SPISettings* m_spiSettings;
};