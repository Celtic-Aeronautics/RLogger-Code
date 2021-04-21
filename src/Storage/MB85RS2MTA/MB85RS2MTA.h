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
    uint8_t m_chipSelect;
    SPIClass* m_spi;
};