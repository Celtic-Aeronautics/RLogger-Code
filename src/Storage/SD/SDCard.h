#pragma once

#include <stdint.h>

class SdFat;
class SdFile;
class Print;

class SDCard
{
public:
    SDCard();

    bool Init(uint8_t chipSelect);

    bool FileExists(const char* path);

    Print* CreateFile(const char* path);

    void CloseFile();

    void TestWrite();

private:  
    uint8_t m_chipSelect;
    SdFat* m_sd;
    SdFile* m_file;
    bool m_open;
};