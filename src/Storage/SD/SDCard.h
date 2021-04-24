#include <stdint.h>

class SDCard
{
public:
    SDCard();

    bool Init(uint8_t chipSelect);

private:  
    uint8_t m_chipSelect;
};