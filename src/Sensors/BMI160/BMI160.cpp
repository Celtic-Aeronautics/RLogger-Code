#include "BMI160.h"

#include <Arduino.h>
#include <Wire.h>

#define BMI_DEBUG_OUT    0
#define BMI_EXTRA_CHECKS 0

const uint8_t k_deviceID = 0xD1;

BMI160::BMI160()
    : m_wire(nullptr)
    , m_address(0x0)
    , m_accPowerMode(AccPowerMode::Suspended)
    , m_accODR(AccODR::ODR_100_HZ)
    , m_accRange(AccRange::RANGE_2_G)
    , m_gyrPowerMode(GyroPowerMode::Suspended)
    , m_gyrODR(GyrODR::ODR_100_HZ)
    , m_gyrRange(GyrRange::RANGE_2000_DPS)
{
}

bool BMI160::Init(TwoWire* wire, uint8_t address)
{
    m_wire = wire;
    m_address = address;

    if(!m_wire || !IsConnected())
    {
        return false;
    }

    // Sofreset
    m_wire->beginTransmission(m_address);
    m_wire->write((uint8_t)Registers::CMD);
    m_wire->write((uint8_t)CMDCodes::softreset);
    m_wire->endTransmission();
    delay(1);

    // Check device ID
#if BMI_EXTRA_CHECKS == 1
    m_wire->beginTransmission(m_address);
    m_wire->write((uint8_t)Registers::CHIPID);
    m_wire->endTransmission();
    if(m_wire->requestFrom(m_address, 1u) == 1u)
    {
        uint8_t deviceID = 0;
        m_wire->readBytes(&deviceID, 1);
        if(deviceID != k_deviceID)
        {
            Serial.println("Invalid ID returned by the device");
            return false;
        }
    }
#endif

    SetPowerMode(AccPowerMode::Normal, GyroPowerMode::Normal);
    
    return true;
}

bool BMI160::IsConnected()
{
    m_wire->beginTransmission(m_address);
    return m_wire->endTransmission() == 0;
}

bool BMI160::ReadIMU()
{
    Vec3 rate = {};
    Vec3 accel = {};
    if(!ReadData(rate, accel))
    {
#if BMI_DEBUG_OUT == 1
        Serial.println("Could not read gyro data");
#endif
        return false;
    }

#if BMI_DEBUG_OUT == 1
    //Serial.print(rate.x, 8);Serial.print(",");Serial.print(rate.y, 8);Serial.print(",");Serial.println(rate.z, 8);
    Serial.print(accel.x, 8);Serial.print(",");Serial.print(accel.y, 8);Serial.print(",");Serial.println(accel.z, 8);
#endif

    return true;
}

bool BMI160::ReadData(Vec3& angularRate, Vec3& acceleration)
{
    // Start reading the gyro data
    int16_t rawGyro[3];
    if(!ReadRawData((uint8_t)Registers::DATA_8, rawGyro, 6u))
    {
        return false;
    }

    // Now read the accelerometer data
    int16_t rawAccel[3];
    if(!ReadRawData((uint8_t)Registers::DATA_14, rawAccel, 6u))
    {
        return false;
    }   

    // Process the data
    angularRate.x = rawGyro[0];
    angularRate.y = rawGyro[1];
    angularRate.z = rawGyro[2];

    const float accRange = GetAccRangeMult(m_accRange);
    acceleration.x = (float)rawAccel[0] / 32767.0f * accRange;
    acceleration.y = (float)rawAccel[1] / 32767.0f * accRange;
    acceleration.z = (float)rawAccel[2] / 32767.0f * accRange;
    
    return true;
}

bool BMI160::ReadRawData(uint8_t dataRegister, int16_t* data, uint8_t count)
{
    m_wire->beginTransmission(m_address);
    m_wire->write(dataRegister);
    m_wire->endTransmission();

    if(m_wire->requestFrom(m_address, count) != count)
    {
        return false;
    }

    for(uint8_t i = 0; i < count; ++i)
    {
        data[i]  = (uint8_t)m_wire->read();
        data[i] |= (uint8_t)m_wire->read() << 8;
    }

    return true;
}

void BMI160::SetPowerMode(AccPowerMode accPM, GyroPowerMode gyrPM)
{
    // [2.11.38]

    // Accelerometer
    if(m_accPowerMode != accPM)
    {
        m_accPowerMode = accPM;

        m_wire->beginTransmission(m_address);
        m_wire->write((uint8_t)Registers::CMD);
        m_wire->write((uint8_t)CMDCodes::acc_set_pmu_mode | (uint8_t)m_accPowerMode);  
        m_wire->endTransmission();
        delay(4);        
    }

    // Gyroscope
    if(m_gyrPowerMode != gyrPM)
    {
        m_gyrPowerMode = gyrPM;
        
        m_wire->beginTransmission(m_address);
        m_wire->write((uint8_t)Registers::CMD);
        m_wire->write((uint8_t)CMDCodes::gyr_set_pmu_mode | (uint8_t)m_gyrPowerMode);
        m_wire->endTransmission();
        delay(81); // Worth checking this
    }
}

float BMI160::GetAccRangeMult(AccRange range)const
{
    switch (range)
    {
        case AccRange::RANGE_2_G:   return 2.0f;
        case AccRange::RANGE_4_G:   return 4.0f;
        case AccRange::RANGE_8_G:   return 8.0f;
        case AccRange::RANGE_16_G:  return 16.0f;
        default:                    return 1.0f;
    }
}

float BMI160::GetGyroRangeMult(GyrRange range)const
{
    switch (range)
    {
        case GyrRange::RANGE_125_DPS:   return 125.0f;
        case GyrRange::RANGE_250_DPS:   return 250.0f;
        case GyrRange::RANGE_500_DPS:   return 500.0f;
        case GyrRange::RANGE_1000_DPS:  return 1000.0f;
        case GyrRange::RANGE_2000_DPS:  return 2000.0f;
        default:                        return 1.0f;
    }
}