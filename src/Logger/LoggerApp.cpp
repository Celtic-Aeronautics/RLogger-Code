#include "LoggerApp.h"

#include "Sensors/BMI160/BMI160.h"
#include "Sensors/MS5611/MS5611.h"

#include "Storage/MB85RS2MTA/MB85RS2MTA.h"
#include "Storage/SD/SDCard.h"

#include "Utils/Pressure.h"
#include "Utils/Debug/DebugOutput.h"

#include <Wire.h>
#include <SPI.h>

#define FRAM_CS 10
#define SD_CS   9

void SetputPinAsCS(uint8_t pin, bool disable = true)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, disable ? HIGH : LOW);
}

LoggerApp::LoggerApp()
    : m_state(LoggerState::Boot)
    , m_imu(nullptr)
    , m_baro(nullptr)
    , m_fram(nullptr)
    , m_sd(nullptr)
{
}

LoggerResult LoggerApp::Init()
{
    // Init data protocols
    Wire.begin();
    SPI.begin();

    // Configure CS Pins
    SetputPinAsCS(FRAM_CS);
    SetputPinAsCS(SD_CS);

    m_imu = new BMI160();
    if(!m_imu->Init(&Wire))
    {
        DEBUG_LOG("Failed to init the IMU");
        return LoggerResult::FailedInitIMU;
    }

    m_baro = new MS5611();
    if(!m_baro->Init(&Wire, MS5611::m_defaultAddr))
    {
        DEBUG_LOG("Failed to initialize baro!");
        return LoggerResult::FailedInitBarometer;
    }

    m_fram = new MB85RS2MTA();
    if(!m_fram->Init(FRAM_CS, &SPI))
    {
        DEBUG_LOG("Failed to initialize the FRAM!");
        return LoggerResult::FailedInitFRAM;
    }

    m_sd = new SDCard();
    if(!m_sd->Init(SD_CS))
    {
        DEBUG_LOG("Failed to init the SD card");
        return LoggerResult::FailedInitSD;
    }

    return LoggerResult::Success;
}

void LoggerApp::Update()
{
    float p = 0.0f;
    m_baro->ReadPressure(p, OSR::OSR_4096, OSR::OSR_4096);
    p = Pressure::MBarToPascal(p);

    float altitude = Pressure::GetAltitudeFromPa(p, 101500.0f);
    float temp = m_baro->GetLastTemperature();
  
    DEBUG_LOG("Altitude: %f ", altitude);

    m_imu->ReadIMU();
}