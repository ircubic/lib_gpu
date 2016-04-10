#pragma once

#include "nvidia_interface_datatypes.h"

extern "C" {
    typedef enum {
        GPU_OVERCLOCK_SETTING_AREA_CORE,
        GPU_OVERCLOCK_SETTING_AREA_MEMORY,
        GPU_OVERCLOCK_SETTING_AREA_SHADER,
        GPU_OVERCLOCK_SETTING_AREA_OVERVOLT
    } GPU_OVERCLOCK_SETTING_AREA;

    struct GpuOverclockSetting
    {
        bool editable;
        float currentValue;
        float minValue;
        float maxValue;
        GpuOverclockSetting();
        GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, const bool editable = false);
    };

    struct GpuOverclockProfile
    {
        const GpuOverclockSetting& operator[](const GPU_OVERCLOCK_SETTING_AREA area);
        GpuOverclockSetting coreOverclock;
        GpuOverclockSetting memoryOverclock;
        GpuOverclockSetting shaderOverclock;
        GpuOverclockSetting overvolt;
    };

    struct GpuUsage
    {
        float coreUsage;
        float fbUsage;
        float vidUsage;
        float busUsage;
    };

    struct GpuClocks
    {
        float coreClock;
        float memoryClock;
        float shaderClock;
    };
}