#pragma once

#include "helpers.h"
#include "nvidia_interface_datatypes.h"
#include "GpuDatatypes.h"

#ifdef __cplusplus
namespace lib_gpu {
namespace nvidia_simple_api {
extern "C" {
#endif

    NVLIB_EXPORTED bool init_simple_api();
    NVLIB_EXPORTED unsigned int get_gpu_count();

    NVLIB_EXPORTED bool get_name(unsigned int gpu_index, char name[NVIDIA_SHORT_STRING_SIZE]);
    NVLIB_EXPORTED struct GpuClocks get_clocks(unsigned int gpu_index);
    NVLIB_EXPORTED struct GpuUsage get_usages(unsigned int gpu_index);
    NVLIB_EXPORTED struct GpuOverclockProfile get_overclock_profile(unsigned int gpu_index);

    NVLIB_EXPORTED bool overclock(unsigned int gpu_index, unsigned int clock, float new_delta);

#ifdef __cplusplus
}
}
}
#endif