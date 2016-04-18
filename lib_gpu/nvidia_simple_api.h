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
    NVLIB_EXPORTED unsigned get_gpu_count();
    NVLIB_EXPORTED unsigned get_index_for_GPUID(unsigned long GPUID);

    NVLIB_EXPORTED bool get_name(unsigned gpu_index, char name[NVIDIA_SHORT_STRING_SIZE]);

    NVLIB_EXPORTED unsigned long getGPUID(unsigned gpu_index);
    NVLIB_EXPORTED float getVoltage(unsigned gpu_index);
    NVLIB_EXPORTED float getTemperature(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuClocks get_clocks(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuClocks get_default_clocks(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuClocks get_base_clocks(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuClocks get_boost_clocks(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuUsage get_usages(unsigned gpu_index);
    NVLIB_EXPORTED struct GpuOverclockProfile get_overclock_profile(unsigned gpu_index);

    NVLIB_EXPORTED bool overclock(unsigned gpu_index, unsigned clock, float new_delta);

#ifdef __cplusplus
}
}
}
#endif