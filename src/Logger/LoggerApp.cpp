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
    , m_currentState()
    , m_prevState()
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

    // Init sensors and storage
    {
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
    }

    // Check status (brown out)
    // TODO

    m_state = LoggerState::Idle; // We are now waiting to detect launch

    return LoggerResult::Success;
}

static bool k_first = true;

void LoggerApp::Update()
{
    GatherCurrentState();

    // Ensure we have valid previous state
    if(k_first)
    {
        m_prevState = m_currentState;
    }

    switch (m_state)
    {
        case LoggerState::Boot:
        {
            break;
        }
        case LoggerState::Idle:
        {
            float deltaAltitude = m_currentState.m_altitude - m_prevState.m_altitude;
            if(deltaAltitude >= 0.2f && m_currentState.m_acceleration.y > 10.0f)
            {
                m_state = LoggerState::Active;
            }
            break;
        }
        case LoggerState::Active:
        {
            break;
        }
        case LoggerState::Dump:
        {
            break;
        }
        case LoggerState::End:
        {
            break;
        }
        case LoggerState::Error:
        {
            break;
        }
    }

    // Swap state for the next frame
    SwapState();
}

void LoggerApp::SwapState()
{
    m_prevState = m_currentState;
}

void LoggerApp::GatherCurrentState()
{
    // Get barometric altitude
    float curPressure = 0.0f;
    m_baro->ReadPressure(curPressure, OSR::OSR_4096, OSR::OSR_4096);
    curPressure = Pressure::MBarToPascal(curPressure);

    m_currentState.m_altitude = Pressure::GetAltitudeFromPa(curPressure, 101500.0f);

    m_currentState.m_temperature = m_baro->GetLastTemperature();

    m_imu->ReadIMU(m_currentState.m_acceleration, m_currentState.m_angularRate);
}