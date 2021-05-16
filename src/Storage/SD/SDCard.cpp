#include "SDCard.h"

#include "Utils/Debug/DebugOutput.h"

#include <SdFat.h>

 // #define SD_TEST_ENABLE

SDCard::SDCard()
    : m_chipSelect(0)
    , m_sd(nullptr)
    , m_file(nullptr)
    , m_open(false)
{
}

bool SDCard::Init(uint8_t chipSelect)
{    
    m_chipSelect = chipSelect;
    m_sd = new SdFat();

    if(!m_sd->begin(chipSelect, SPI_HALF_SPEED))
    {
        return false;
    }
    
    return true;
}

bool SDCard::FileExists(const char* path)
{
    return m_sd->exists(path);
}

Print* SDCard::CreateFile(const char* path)
{
    // Don't allow more than one file
    if(m_open)
    {
        return nullptr;
    }

    // Create it the first time
    if(!m_file)
    {
        m_file = new SdFile;
    }

    if(!m_file->open(path, O_RDWR | O_CREAT | O_TRUNC))
    {
        m_sd->errorPrint(&Serial);
        return nullptr;
    }

    m_open = true;

    // SdFile inherits from Print (that's what we are mainly interested in)
    return m_file;
}

void SDCard::CloseFile()
{
    if(m_file)
    {
        m_file->close();
        m_open = false;
    }
}

void SDCard::TestWrite()
{
#ifdef SD_TEST_ENABLE
    // NOTE: calling open with only O_CREAT will 'work' but the return will be false
    //       adding O_RDWR makes it work. This is a bit weird...     
    SdFile file;
    if(file.open("log5.csv", O_RDWR | O_CREAT | O_TRUNC))
    {
        DEBUG_LOG("Win win");
        file.print(42);
        file.close();
    }
    else
    {
        m_sd->errorPrint(&Serial);
        DEBUG_LOG("Failed to run the SD card Test %i", file.getError());
    }
#endif
}