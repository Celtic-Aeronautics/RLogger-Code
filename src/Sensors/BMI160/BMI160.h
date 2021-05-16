#pragma once

#include <stdint.h>

#include "RMath.h"

class TwoWire;

enum class AccODR : uint8_t
{
    ODR_25_HZ = 6,
    ODR_50_HZ,
    ODR_100_HZ,
    ODR_200_HZ,
    ODR_400_HZ,
    ODR_800_HZ,
    ODR_1600_HZ,
};

enum class AccRange : uint8_t
{
    RANGE_2_G  = 3,
    RANGE_4_G  = 5,
    RANGE_8_G  = 8,
    RANGE_16_G = 12,
};

enum class GyrODR : uint8_t
{
    ODR_25_HZ = 6,
    ODR_50_HZ,
    ODR_100_HZ,
    ODR_200_HZ,
    ODR_400_HZ,
    ODR_800_HZ,
    ODR_1600_HZ,
    ODR_3200_HZ
};

enum class GyrRange : uint8_t
{
    RANGE_2000_DPS = 0,
    RANGE_1000_DPS = 1,
    RANGE_500_DPS  = 2,
    RANGE_250_DPS  = 3,
    RANGE_125_DPS  = 4,
};

// IMU BMI160
// Datasheet: https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bmi160-ds000.pdf
class BMI160
{
public:
    BMI160();
    
    bool Init(TwoWire* wire, uint8_t address = 0x69);

    bool IsConnected();

    bool ReadIMU(Vec3& acceleration, Vec3& angularRate);

private:
    
    bool ReadData(Vec3& acceleration, Vec3& angularRate);

    bool ReadRawData(uint8_t dataRegister, int16_t* data, uint8_t count);

    enum class Registers : uint8_t
    {
        CHIPID      = 0x00,
        ERR_REG     = 0x02,
        PMU_STATUS  = 0x03,
        DATA_8      = 0x0C, // Gyro data start
        DATA_14     = 0x12, // Acc data start
        ACC_CONF    = 0x40,
        ACC_RANGE   = 0x41,
        GYR_CONF    = 0x42,
        GYR_RANGE   = 0x43,
        CMD         = 0x7E,        
    };

    enum class AccPowerMode : uint8_t
    {
        Suspended   = 0u,
        Normal      = 1u,
        Low         = 2u,
    };

    enum class GyroPowerMode : uint8_t
    {
        Suspended   = 0u,
        Normal      = 1u,
        FastStartUp = 3u,
    };

    enum class CMDCodes : uint8_t
    {
        start_foc           = 0x3,
        acc_set_pmu_mode    = 0x10,
        gyr_set_pmu_mode    = 0x14,
        mag_set_pmu_mode    = 0x18,
        prog_nvm            = 0xA0,
        fifo_flush          = 0xB0,
        int_reset           = 0xB1,
        softreset           = 0xB6,
        step_cnt_clr        = 0xB2,
    };

    void SetPowerMode(AccPowerMode accPM, GyroPowerMode gyrPM);

    float GetAccRangeMult(AccRange range)const;

    float GetGyroRangeMult(GyrRange range)const;
    
    TwoWire* m_wire;
    uint8_t m_address;

    AccPowerMode m_accPowerMode;
    AccODR m_accODR;        // 10 HZ default
    AccRange m_accRange;    // +- 2G default

    GyroPowerMode m_gyrPowerMode;
    GyrODR m_gyrODR;        // 100HZ default
    GyrRange m_gyrRange;    // +- 2000DPS default
};