#pragma once

#include <stdint.h>

class SDCard
{
public:
    SDCard();

    bool Init(uint8_t chipSelect);

    void TestWrite();

private:  
    uint8_t m_chipSelect;
};