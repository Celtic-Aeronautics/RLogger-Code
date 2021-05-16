#pragma once

#include <stdint.h>

struct MedianFilter
{
    MedianFilter();

    // Adds a signal value and returns the accumulated median
    float ProcessEntry(float value);

private:
    static const uint8_t k_windowLen = 5;
    float m_window[k_windowLen];
    uint8_t m_windowIndex;
};