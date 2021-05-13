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
            return LoggerResult::FailedInitIMU;
        }

        m_baro = new MS5611();
        if(!m_baro->Init(&Wire, MS5611::m_defaultAddr))
        {
            return LoggerResult::FailedInitBarometer;
        }

#ifndef DISABLE_FRAM
        m_fram = new MB85RS2MTA();
        if(!m_fram->Init(FRAM_CS, &SPI))
        {
            return LoggerResult::FailedInitFRAM;
        }
#endif

        m_sd = new SDCard();
        if(!m_sd->Init(SD_CS))
        {
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

    GatherCurrentState(0.0f);

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
    
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);

    while(true)
    {    
        uint64_t startTime = millis();  

        if(elapsed >= m_deltaTime)
        {
            elapsed = 0.0f; // Reset elapsed
        
            GatherCurrentState(totalTimeSec);

            // Write state to the FRAM and increment the address
            m_fram->Write(m_currentFRAMAddr, (uint8_t*)&m_currentState, m_stateDataSize);
            m_currentFRAMAddr += m_stateDataSize;
            ++numSamples;
        }
        else
        {
            // delay(2);
        }
        

        // Update elapsed
        uint64_t curTime = millis();
        float deltaTime = (float)(curTime - startTime) * 0.001f;

        elapsed += deltaTime;
        totalTimeSec += deltaTime;

        // Stop if we are done for the test
        if(totalTimeSec >= runTime)
        {
            break;
        }
    }

    digitalWrite(LED_BUILTIN, LOW);

    // Dump to the SD card
    Print* file = m_sd->CreateFile("RunTest.csv");
    if(!file)
    {
        return;
    }
    SerializeHeader(file);
    State parsedState = {};
    for(uint32_t sampleIdx = 0; sampleIdx < numSamples; ++sampleIdx)
    {
        m_fram->Read(sampleIdx * m_stateDataSize, (uint8_t*)&parsedState, m_stateDataSize);
        SerializeState(parsedState, file);
    }
    m_sd->CloseFile();

    digitalWrite(LED_BUILTIN, HIGH);
}

void LoggerApp::SwapState()
{
    m_prevState = m_currentState;
}

void LoggerApp::GatherCurrentState(float time)
{
    m_currentState.m_timeStamp = time;

    // Get barometric altitude
    float curPressure = 0.0f;
    m_baro->ReadPressure(curPressure, OSR::OSR_4096, OSR::OSR_4096);
    curPressure = Pressure::MBarToPascal(curPressure);

    m_currentState.m_altitude = Pressure::GetAltitudeFromPa(curPressure, 101500.0f);

    m_currentState.m_temperature = m_baro->GetLastTemperature();

    m_imu->ReadIMU(m_currentState.m_acceleration, m_currentState.m_angularRate);
}

void LoggerApp::SerializeHeader(Print* stream)
{
    stream->print(F("TIME, ALTITUDE, TEMP, ACCEL_X, ACCEL_Y, ACCEL_Z, RATE_X, RATE_Y, RATE_Z \n"));
}

void LoggerApp::SerializeState(const State& state, Print* stream)
{
    stream->print(state.m_timeStamp); 
    stream->print(',');
    stream->print(state.m_altitude); 
    stream->print(',');
    stream->print(state.m_temperature); 
    stream->print(',');
    stream->print(state.m_acceleration.x); 
    stream->print(',');
    stream->print(state.m_acceleration.y); 
    stream->print(',');
    stream->print(state.m_acceleration.z); 
    stream->print(',');
    stream->print(state.m_angularRate.x); 
    stream->print(',');
    stream->print(state.m_angularRate.y); 
    stream->print(',');
    stream->print(state.m_angularRate.z);
    stream->print('\n');
}