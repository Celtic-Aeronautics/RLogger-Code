#include <Arduino.h>

#include "Logger/LoggerApp.h"

LoggerApp g_app;

void setup() 
{
#ifdef DEBUG_OUTPUT_ENABLED
  Serial.begin(9600);
  while(!Serial) {};
#endif

  g_app.Init();
}

void loop() 
{
  g_app.Update();

  delay(50);
}