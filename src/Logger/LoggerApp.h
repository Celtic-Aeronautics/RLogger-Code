#pragma once

#include <stdint.h>

#include "Utils/Math.h"

enum class LoggerState : uint8_t
{
    Boot,
    Idle,
    Active,
    Dump,
    End,
    Error,
    COUNT
};

enum class LoggerResult : uint8_t
{
    Success,
    FailedInitBarometer,
    FailedInitIMU,
    FailedInitFRAM,
    FailedInitSD,
    COUNT,
};

struct State
{
    float m_altitude;
    float m_temperature;
    Vec3 m_acceleration;
    Vec3 m_angularRate;
};

class BMI160;
class MS5611;
class MB85RS2MTA;
class SDCard;

class LoggerApp
{
public:
    LoggerApp();

    LoggerResult Init();

    void Update();

private:
    LoggerState m_state;

    BMI160* m_imu;
    MS5611* m_baro;
    MB85RS2MTA* m_fram;
    SDCard* m_sd;
};