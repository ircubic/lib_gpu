#pragma once

#include "helpers.h"
#include "GpuDatatypes.h"

extern "C" {
    struct nvidia_simple_clocks
    {
        float coreClock;
        float memoryClock;
        float shaderClock;
    };

    NVLIB_EXPORTED bool init_simple_api();
    NVLIB_EXPORTED struct nvidia_simple_clocks get_clocks();
    NVLIB_EXPORTED struct GpuUsage get_usages();
    NVLIB_EXPORTED struct GpuOverclockProfile get_overclock_profile();
    NVLIB_EXPORTED bool overclock(float new_delta, int clock);
}