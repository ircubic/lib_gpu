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

    struct NVLIB_EXPORTED nvidia_simple_overclock_setting
    {
        float currentValue;
        float minValue;
        float maxValue;
    };

    struct NVLIB_EXPORTED nvidia_simple_overclock_profile
    {
        struct nvidia_simple_overclock_setting coreOverclock;
    };

    NVLIB_EXPORTED bool init_simple_api();
    NVLIB_EXPORTED struct nvidia_simple_clocks get_clocks();
    NVLIB_EXPORTED struct nvidia_simple_usages get_usages();
    NVLIB_EXPORTED struct nvidia_simple_overclock_profile get_overclock_profile();
    NVLIB_EXPORTED bool overclock(float new_delta, int clock);
}