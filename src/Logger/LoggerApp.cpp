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

// #define DISABLE_FRAM

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
    , m_samplesPerSecond(0)
    , m_deltaTime(0.0f)
    , m_stateDataSize((uint8_t)sizeof(State))
    , m_currentFRAMAddr(0)
{
}

LoggerResult LoggerApp::Init(int samplesPerSecond)
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

#ifndef DISABLE_FRAM
        m_fram = new MB85RS2MTA();
        if(!m_fram->Init(FRAM_CS, &SPI))
        {
            DEBUG_LOG("Failed to initialize the FRAM!");
            return LoggerResult::FailedInitFRAM;
        }
#endif

        m_sd = new SDCard();
        if(!m_sd->Init(SD_CS))
        {
            DEBUG_LOG("Failed to init the SD card");
            return LoggerResult::FailedInitSD;
        }

        m_sd->TestWrite();
    }

    // Setup delta times
    m_samplesPerSecond = samplesPerSecond;
    m_deltaTime = 1.0f / (float)m_samplesPerSecond;

    // Check status (brown out)
    // TODO

    m_state = LoggerState::Idle; // We are now waiting to detect launch

    return LoggerResult::Success;
}

static bool k_first = true;

void LoggerApp::Run()
{
    // TO-DO: while true, take time

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

// TO-DO: ifdef this out, only for testing
void LoggerApp::RunTest(float runTime)
{
    float totalTimeSec = 0.0f;
    float elapsed = m_deltaTime;
    uint32_t numSamples = 0;
    
    while(true)
    {    
        uint64_t startTime = millis();  

        if(elapsed >= m_deltaTime)
        {
            elapsed = 0.0f;
            
            GatherCurrentState();

            // Write state to the FRAM and increment the address
            m_fram->Write(m_currentFRAMAddr, (uint8_t*)&m_currentState, m_stateDataSize);
            m_currentFRAMAddr += m_stateDataSize;
            ++numSamples;
        }
        else
        {
            // Wait for 50% of the remaining time (this will spin.. maybe we can do something useful here)
            float toWait = (m_deltaTime - elapsed) * 0.5f;
            delay(toWait);
        }

        // Update elapsed
        uint64_t curTime = millis();
        uint32_t deltaTime = curTime - startTime;

        elapsed += deltaTime * 0.001f;
        totalTimeSec += elapsed;

        // Stop if we are done for the test
        if(totalTimeSec >= runTime)
        {
            break;
        }
    }

    // Dump to the SD card
    for(uint32_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
    {

    }
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