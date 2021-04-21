#pragma once

#include <stdint.h>

class SPIClass;

// Memory FRAM
// Datasheet: https://cdn-shop.adafruit.com/product-files/4718/4718_MB85RS2MTA.pdf
class MB85RS2MTA
{
public:
    MB85RS2MTA();

    bool Init(uint8_t chipSelect, SPIClass* spi);

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

    uint8_t m_chipSelect;
    SPIClass* m_spi;
};