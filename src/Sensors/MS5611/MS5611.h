#pragma once

#include <stdint.h>

class TwoWire;

enum class OSR : uint8_t
{
    OSR_256,
    OSR_512,
    OSR_1024,
    OSR_2048,
    OSR_4096,
};

enum class DType : uint8_t
{
    D_PRESSURE,
    D_TEMPERATURE,
};

// Pressure sensor MS5611
// Range: 10mbar to 1200mbar
//       -40C to 85C
// Datasheet: https://www.te.com/commerce/DocumentDelivery/DDEController?Action=showdoc&DocId=Data+Sheet%7FMS5611-01BA03%7FB3%7Fpdf%7FEnglish%7FENG_DS_MS5611-01BA03_B3.pdf%7FCAT-BLPS0036
class MS5611
{
public:
    MS5611();
    
    ~MS5611();

    MS5611(const MS5611& other) = delete;

    bool Init(TwoWire* wire, uint8_t address);

    bool IsConnected();

    void Reset();

    // Gives pressure in mbar
    bool ReadPressure(float& pressure, OSR tempOSR = OSR::OSR_512, OSR pressureOSR = OSR::OSR_512);

    // After succesfully running ReadPressure, this returns last valid temperature in C
    float GetLastTemperature()const;

    static const uint8_t m_defaultAddr = 0x77;

private:
    void WriteCommand(uint8_t command);

    // Sends and waits based on the appropiate data type and OSR
    void RequestConversionAndWait(DType type, OSR osr);

    // Sends the ADC command internally
    bool ReadADC(uint32_t& value);

    bool ReadCalibration();

    // User must send the command outside
    bool ReadCalibrationValue(uint32_t& value);

    TwoWire* m_wire;
    uint8_t m_address;
    uint32_t m_calibration[7];
    float m_lastTemperature;
};