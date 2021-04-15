#include <Arduino.h>
#include <Wire.h>

#include "Sensors/MS5611/MS5611.h"
#include "Sensors/BMI160/BMI160.h"

#include "Utils/Pressure.h"

MS5611 pressureSensor;
BMI160 imu;

// Would be nice to have a base sensor class

void setup() 
{
  Serial.begin(9600);
  Wire.begin();

  if(!imu.Init(&Wire))
  {
    Serial.println("Failed to init the IMU");
  }

  if(!pressureSensor.Init(&Wire, MS5611::m_defaultAddr))
  {
    Serial.println("Failed to initialize the pressure sensor!");
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