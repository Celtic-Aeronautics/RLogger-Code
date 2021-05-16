#pragma once

class Pressure
{
public:

    static float MBarToPascal(float mbar);

    // Returns the altitude in meters 
    static float GetAltitudeFromPa(float pressure, float pressureAtSeaLevel);
};