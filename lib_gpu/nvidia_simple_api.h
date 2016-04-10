#pragma once

#include "helpers.h"
#include "GpuDatatypes.h"

extern "C" {
    NVLIB_EXPORTED bool init_simple_api();
    NVLIB_EXPORTED struct GpuClocks get_clocks();
    NVLIB_EXPORTED struct GpuUsage get_usages();
    NVLIB_EXPORTED struct GpuOverclockProfile get_overclock_profile();
    NVLIB_EXPORTED bool overclock(float new_delta, int clock);
}