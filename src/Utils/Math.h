#pragma once

#include <math.h>

struct Vec3
{
    float x;
    float y;
    float z;
};

inline float Length(const Vec3& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}