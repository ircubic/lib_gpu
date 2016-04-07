#pragma once

#include "helpers.h"
extern "C" {
    struct nvidia_simple_clocks
    {
        float coreClock;
        float memoryClock;
        float shaderClock;
    };

    struct nvidia_simple_usages
    {
        float gpuUsage;
        float fbUsage;
        float vidUsage;
        float busUsage;
    };

    struct nvidia_simple_overclock_setting
    {
        float currentValue;
        float minValue;
        float maxValue;
    };

    struct nvidia_simple_overclock_profile
    {
        struct nvidia_simple_overclock_setting coreOverclock;
        struct nvidia_simple_overclock_setting memoryOverclock;
        struct nvidia_simple_overclock_setting shaderOverclock;
    };

    NVLIB_EXPORTED bool init_simple_api();
    NVLIB_EXPORTED struct nvidia_simple_clocks get_clocks();
    NVLIB_EXPORTED struct nvidia_simple_usages get_usages();
    NVLIB_EXPORTED struct nvidia_simple_overclock_profile get_overclock_profile();
    NVLIB_EXPORTED bool overclock(float new_delta, int clock);
}