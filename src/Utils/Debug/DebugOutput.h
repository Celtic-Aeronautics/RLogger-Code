#pragma once

#ifdef DEBUG_OUTPUT_ENABLED

    void MsgOutImpl(const char* msg, ...);

    #define DEBUG_LOG(msg, ...) MsgOutImpl(msg, ##__VA_ARGS__)

#else
    
    #define DEBUG_LOG(msg, ...)

#endif