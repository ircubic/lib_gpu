#include "pch.h"
#include "GpuDatatypes.h"

#define LIFT_UNIT(x) (x/1000.0)

const GpuOverclockSetting& GpuOverclockProfile::operator[](GPU_OVERCLOCK_SETTING_AREA area)
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
    }

    return this->coreOverclock;
}

GpuOverclockSetting::GpuOverclockSetting()
{
    this->editable = false;
    this->currentValue = this->maxValue = this->minValue = 0.0;
}

GpuOverclockSetting::GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, bool editable)
{
    this->editable = editable;
    this->currentValue = LIFT_UNIT(delta.value);
    this->minValue = LIFT_UNIT(delta.val_min);
    this->maxValue = LIFT_UNIT(delta.val_max);
}