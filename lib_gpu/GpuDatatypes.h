#pragma once

#include "nvidia_interface_datatypes.h"

typedef float MHz;
typedef float mV;

template <typename T>
struct GpuOverclockSetting
{
    bool editable;
    T currentValue;
    T minValue;
    T maxValue;
    GpuOverclockSetting();
    GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, bool editable = false);
};

extern "C" struct GpuOverclockProfile
{
    GpuOverclockSetting<MHz> coreOverclock;
    GpuOverclockSetting<MHz> memoryOverclock;
    GpuOverclockSetting<MHz> shaderOverclock;
    GpuOverclockSetting<mV> overvolt;
};

struct GpuUsage
{
    float coreUsage;
    float fbUsage;
    float vidUsage;
    float busUsage;
};