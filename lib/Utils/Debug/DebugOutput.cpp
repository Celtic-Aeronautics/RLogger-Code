#include "DebugOutput.h"

#include <Arduino.h>

#if DEBUG_OUTPUT_ENABLED == 1

void MsgOutImpl(const char* msg, ...)
{
    char buff[256];
	va_list argptr;
	va_start(argptr, msg);
	vsnprintf(buff, 256, msg, argptr);
    Serial.println(buff);
}

#endif