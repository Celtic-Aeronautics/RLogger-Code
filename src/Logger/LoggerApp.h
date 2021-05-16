#pragma once

#include <stdint.h>

#include "RMath.h"
#include "Filters.h"

#include "LoggerDefinitions.h"

class BMI160;
class MS5611;
class MB85RS2MTA;
class SDCard;
class Print;

class LoggerApp
{
public:
    LoggerApp();

    LoggerResult Init(int samplesPerSecond);

    void Run();

    // For testing only. It will record data for 'runTime' and dump it to the SD card
    // frequency as defined by Init()
    void RunTest(float runTime);

private:
    void Update(float totalTimeSec, float deltaTime);

    void SwapState();

    void GatherCurrentState(float time);

    void SerializeHeader(Print* stream);

    void SerializeItem(Print* stream, float value, char separator, bool printSeparator = true);

    void SerializeState(const State& state, Print* stream);

    LoggerState m_state;

    BMI160* m_imu;
    MS5611* m_baro;
    MB85RS2MTA* m_fram;
    SDCard* m_sd;

    State m_currentState;
    State m_prevState;

    // The number of states to capture per second
    int m_samplesPerSecond;

    // Time in seconds between each sample (during Active state)
    float m_deltaTimeActive;

    // Requested delta time (can change based on the current state)
    float m_targetDeltaTime;

    // The size (in bytes) of each state packet
    uint8_t m_stateDataSize;

    uint32_t m_currentFRAMAddr;

    uint32_t m_numSamples;

    // How many samples can we store in the FRAM?
    uint32_t m_maxSamples;

    // With the current sampling frequency, for how long can we cample?
    float m_maxActiveTime;

    // The altitude at the liftoff event
    float m_liftOffAltitude;

    // Median filter used to figure out if we landed
    MedianFilter m_landingFilter;
};