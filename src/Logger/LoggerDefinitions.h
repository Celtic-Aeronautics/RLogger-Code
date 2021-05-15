#pragma once

enum class LoggerState : uint8_t
{
    Boot,
    Idle,
    Active,
    Dump,
    End,
    Error,
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
    float m_timeStamp;
    float m_altitude;
    float m_temperature;
    Vec3 m_acceleration;
    Vec3 m_angularRate;
};