#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "Sensors/MS5611/MS5611.h"
#include "Sensors/BMI160/BMI160.h"

#include "Storage/MB85RS2MTA/MB85RS2MTA.h"
#include "Storage/SD/SDCard.h"

#include "Utils/Pressure.h"
#include "Utils/Debug/DebugOutput.h"

MS5611 pressureSensor;
BMI160 imu;
MB85RS2MTA fram;
SDCard sdCard;

#define FRAM_CS 10
#define SD_CS   9

void SetputPinAsCS(uint8_t pin, bool disable = true)
{
  pinMode(pin, OUTPUT);
  digitalWrite(pin, disable ? HIGH : LOW);
}

void setup() 
{
#ifdef DEBUG_OUTPUT_ENABLED
  Serial.begin(9600);
  while(!Serial) {};
#endif

  Wire.begin();
  SPI.begin();

  // Configure CS Pins
  SetputPinAsCS(FRAM_CS);
  SetputPinAsCS(SD_CS);

  if(!imu.Init(&Wire))
  {
    DEBUG_LOG("Failed to init the IMU");
  }

  if(!pressureSensor.Init(&Wire, MS5611::m_defaultAddr))
  {
    DEBUG_LOG("Failed to initialize the pressure sensor!");
  }

  if(!fram.Init(FRAM_CS, &SPI))
  {
    DEBUG_LOG("Failed to initialize the FRAM!");
  }

  if(!sdCard.Init(SD_CS))
  {
    DEBUG_LOG("Failed to init the SD card");
  }
}

void loop() 
{
  float p = 0.0f;
  pressureSensor.ReadPressure(p, OSR::OSR_4096, OSR::OSR_4096);
  p = Pressure::MBarToPascal(p);

  float altitude = Pressure::GetAltitudeFromPa(p, 101500.0f);
  float temp = pressureSensor.GetLastTemperature();
  
  DEBUG_LOG("Altitude: %f ", altitude);

  imu.ReadIMU();

  delay(50);
}