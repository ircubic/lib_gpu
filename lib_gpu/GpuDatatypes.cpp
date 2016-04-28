#include "pch.h"
#include "GpuDatatypes.h"
#include "nvidia_interface_datatypes.h"

#define LIFT_UNIT(x) (x/1000.0f)

namespace lib_gpu {

const GpuOverclockSetting& GpuOverclockProfile::operator[](GPU_OVERCLOCK_SETTING_AREA area) const
{
    switch (area) {
    case GPU_OVERCLOCK_SETTING_AREA_CORE:
        return this->coreOverclock;
    case GPU_OVERCLOCK_SETTING_AREA_MEMORY:
        return this->memoryOverclock;
    case GPU_OVERCLOCK_SETTING_AREA_SHADER:
        return this->shaderOverclock;
    case GPU_OVERCLOCK_SETTING_AREA_OVERVOLT:
        return this->overvolt;
    case GPU_OVERCLOCK_SETTING_AREA_POWER_LIMIT:
        return this->powerLimit;
    }

    return this->coreOverclock;
}

GpuOverclockSetting::GpuOverclockSetting() : GpuOverclockSetting(0.0, 0.0, 0.0, false)
{
}

GpuOverclockSetting::GpuOverclockSetting(float min, float current, float max, bool editable)
{
    this->editable = editable;
    this->minValue = min;
    this->currentValue = current;
    this->maxValue = max;
}

GpuOverclockSetting::GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, bool editable)
    : GpuOverclockSetting(LIFT_UNIT(delta.val_min), LIFT_UNIT(delta.value), LIFT_UNIT(delta.val_max), editable)
{
}

}