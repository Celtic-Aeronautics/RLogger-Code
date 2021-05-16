#pragma once

#include "math.h"

struct Vec3
{
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    float x;
    float y;
    float z;
};

inline float Length(const Vec3& vec)
{
    return sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}