#include "MS5611.h"

#include "Debug/DebugOutput.h"

#include <Arduino.h>
#include <Wire.h>

const uint8_t k_resetDelay = 10; // ms

const uint8_t k_resetCommand = 0x1E;
const uint8_t k_readCommand  = 0xA0; // Starting addr, we can read from 0xA0 to 0xAE
const uint8_t k_adcReadCommand = 0x0;
const uint8_t k_convertD1Commands [5] = {0x40, 0x42, 0x44, 0x46, 0x48 };
const uint8_t k_convertD2Commands [5] = {0x50, 0x52, 0x54, 0x56, 0x58 };

#define MS_TEST      0
#define MS_TEST_LOWT 0

int64_t clamp(int64_t value, int64_t minValue, int64_t maxValue)
{
    return max(min(value, maxValue), minValue);
}

int32_t clamp(int32_t value, int32_t minValue, int32_t maxValue)
{
    return max(min(value, maxValue), minValue);
}

MS5611::MS5611()
    : m_wire(nullptr)
    , m_address(0)
    , m_lastTemperature(0.0f)
{
}

MS5611::~MS5611()
{
}

bool MS5611::Init(TwoWire* wire, uint8_t address)
{
    m_wire = wire;
    m_address = address;
    
    if(!m_wire || !IsConnected())
    {
        return false;
    }

    // Reset after initial Init
    Reset();
    
    return ReadCalibration();
}

bool MS5611::IsConnected()
{
    m_wire->beginTransmission(m_address);
    return m_wire->endTransmission() == 0;
}

void MS5611::Reset()
{
    WriteCommand(k_resetCommand);
    delay(k_resetDelay); // Give some time for the reset to take place
}

bool MS5611::ReadPressure(float& pressure, OSR tempOSR, OSR pressureOSR)
{
#if MS_TEST == 0 && MS_TEST_LOWT == 0
    // Read digital pressure
    uint32_t D1 = 0;
    RequestConversionAndWait(DType::D_PRESSURE, pressureOSR);
    if(!ReadADC(D1))
    {
        return false;
    }

    // Read digital temperature
    uint32_t D2 = 0;
    RequestConversionAndWait(DType::D_TEMPERATURE, tempOSR);
    if(!ReadADC(D2))
    {
        return false;
    }
#else
#if MS_TEST_LOWT != 0
    uint32_t D1 = 9085466; //1000.09 mbar
    uint32_t D2 = 8569150;
#else
    // Test values (TEMP should be 20.07C and P 1000.09 mbar)
    uint32_t D1 = 9085466;
    uint32_t D2 = 8569150;
#endif
#endif

    // Calculate temperature
    int32_t dT = clamp(D2 - m_calibration[5], -16776960, 16777216);
    int32_t TEMP = 2000 + dT * m_calibration[6] / 8388608; // max intermediate size 41 :U

    int32_t T2 = 0;
    int32_t OFF2 = 0;
    int32_t SENS2 = 0;

    if(TEMP < 2000)
    {
        T2 = (dT * dT) / 2147483648;
        int32_t tmp = ((TEMP - 2000) * (TEMP - 2000)) * 5;
        OFF2 = tmp / 2;
        SENS2 = tmp / 4;
        if(TEMP < -1500.0f)
        {
            tmp = (TEMP + 1500) * (TEMP + 1500);
            OFF2 = OFF2 + 7 * tmp;
            SENS2 = SENS2 + 11 * tmp / 2;
        }
    }

    // Adjust TEMP
    TEMP = TEMP - T2;

    // Calculate temperature compensated pressure
    int64_t OFF = clamp(((int64_t)m_calibration[2] + ((int64_t)m_calibration[4] * (int64_t)dT) / 128ll), -8589672450ll, 12884705280ll) - OFF2;
    int64_t SENS = clamp(((int64_t)m_calibration[1] + ((int64_t)m_calibration[3] * (int64_t)dT) / 256ll), -4294836225ll, 6442352640ll) - SENS2;
    int32_t P = ((int64_t)D1 * SENS / 2097152ll - OFF) / 32768ll;

    pressure = (float)P * 0.01f;
    m_lastTemperature = (float)TEMP * 0.01f;

#if 0
    Serial.print("Temperature: "); Serial.print(m_lastTemperature); 
    Serial.print(" Pressure: "); Serial.println(pressure);
#endif

    return true;
}

float MS5611::GetLastTemperature()const
{
    return m_lastTemperature;
}

void MS5611::WriteCommand(uint8_t command)
{
    m_wire->beginTransmission(m_address);
    m_wire->write(command);
    m_wire->endTransmission();
}

void MS5611::RequestConversionAndWait(DType type, OSR osr)
{
    uint8_t osrIndex = (uint8_t)osr;
    WriteCommand(
        type == DType::D_PRESSURE ? k_convertD1Commands[osrIndex] : k_convertD2Commands[osrIndex]
    );
    static const uint8_t delays[] = {1, 2, 3, 5, 10}; // From the data sheet
    delay(delays[osrIndex]);
}

bool MS5611::ReadADC(uint32_t& value)
{
    // NOTE: The sensor stores D1/2 as 24 bit unsigned integers (we read 3 bytes)
    WriteCommand(k_adcReadCommand);
    if(m_wire->requestFrom(m_address, 3u) == 3)
    {
        value  = (uint32_t)m_wire->read() << 16u;
        value |= (uint32_t)m_wire->read() << 8u;
        value |= (uint32_t)m_wire->read();
        return true;
    }
    return false;
}

bool MS5611::ReadCalibration()
{
#if MS_TEST == 0    
    uint8_t command = 0;
    for(uint8_t index = 0; index < 7; ++index)
    {
        command = k_readCommand | (index << 1);
        WriteCommand(command);
        if(!ReadCalibrationValue(m_calibration[index]))
        {
            return false;
        }
        
    }
#else
    // From the datasheet
    m_calibration[0] = 1;
    m_calibration[1] = 40127;
    m_calibration[2] = 36924;
    m_calibration[3] = 23317;
    m_calibration[4] = 23282;
    m_calibration[5] = 33464;
    m_calibration[6] = 28312;
#endif

#if 0
    DEBUG_LOG("Calibration values:");
    for(uint8_t index = 0; index < 7; ++index)
    {
        DEBUG_LOG("%i", m_calibration[index]);
    }
#endif

    
    // Do this only once
    m_calibration[1] *= 32768;
    m_calibration[2] *= 65536;
    m_calibration[5] *= 256;

    return true;
}

bool MS5611::ReadCalibrationValue(uint32_t& value)
{
    if(m_wire->requestFrom(m_address, 2u) == 2)
    {
        value  = (uint32_t)m_wire->read() << 8u;
        value |= (uint8_t)m_wire->read();
        return true;
    }
    return false;
}