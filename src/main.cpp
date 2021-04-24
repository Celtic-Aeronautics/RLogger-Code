#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>

#include "Sensors/MS5611/MS5611.h"
#include "Sensors/BMI160/BMI160.h"

#include "Storage/MB85RS2MTA/MB85RS2MTA.h"
#include "Storage/SD/SDCard.h"

#include "Utils/Pressure.h"

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
  Serial.begin(9600);

  Wire.begin();
  SPI.begin();

  // Configure CS Pins
  SetputPinAsCS(FRAM_CS);
  SetputPinAsCS(SD_CS);

  if(!imu.Init(&Wire))
  {
    Serial.println("Failed to init the IMU");
  }

  if(!pressureSensor.Init(&Wire, MS5611::m_defaultAddr))
  {
    Serial.println("Failed to initialize the pressure sensor!");
  }

  if(!fram.Init(FRAM_CS, &SPI))
  {
    Serial.println("Failed to initialize the FRAM!");
  }

  if(!sdCard.Init(SD_CS))
  {
    Serial.println("Failed to init the SD card");
  }
}

void loop() 
{
  float p = 0.0f;
  pressureSensor.ReadPressure(p, OSR::OSR_4096, OSR::OSR_4096);
  p = Pressure::MBarToPascal(p);

  float altitude = Pressure::GetAltitudeFromPa(p, 101500.0f);
  float temp = pressureSensor.GetLastTemperature();
  
  imu.ReadIMU();

  delay(50);
}