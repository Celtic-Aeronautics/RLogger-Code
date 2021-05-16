#include "Pressure.h"

#include <RMath.h>

float Pressure::MBarToPascal(float mbar)
{
    return mbar * 100.0f;
}

float Pressure::GetAltitudeFromPa(float pressure, float pressureAtSeaLevel)
{
    // https://cdn-shop.adafruit.com/datasheets/BST-BMP180-DS000-09.pdf
    // barometric formula (good for up to 9000m)
    return 44330.0f * (1.0f - pow(pressure / pressureAtSeaLevel , 0.190294f));
}
