#pragma once

#include "helpers.h"
extern "C" {
    struct NVLIB_EXPORTED nvidia_simple_clocks
    {
        float coreClock;
        float memoryClock;
        float shaderClock;
    };

    struct NVLIB_EXPORTED nvidia_simple_usages
    {
        float gpuUsage;
        float fbUsage;
        float vidUsage;
        float busUsage;
    };

    NVLIB_EXPORTED struct nvidia_simple_clocks get_clocks();
    NVLIB_EXPORTED struct nvidia_simple_usages get_usages();
    NVLIB_EXPORTED bool init_simple_api();
}