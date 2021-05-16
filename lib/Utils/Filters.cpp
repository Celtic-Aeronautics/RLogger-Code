#include "Filters.h"

#include <Arduino.h>

MedianFilter::MedianFilter()
    : m_windowIndex(0)
{
    memset(&m_window, 0, sizeof(float) * k_windowLen);
}

float MedianFilter::ProcessEntry(float value)
{
    // Store value and increment index
    m_window[m_windowIndex++] = value;

    // Loop the value so we stomp over the oldest value in the window
    m_windowIndex = m_windowIndex % k_windowLen;

    float acum = 0.0f;
    for(uint8_t i = 0; i < k_windowLen; ++i)
    {
        acum += m_window[i];
    }
    return acum / (float)k_windowLen;
}