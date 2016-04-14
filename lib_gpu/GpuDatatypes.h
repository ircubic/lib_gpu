#pragma once



#ifdef __cplusplus
namespace lib_gpu {
extern "C" {
#endif
    struct NVIDIA_DELTA_ENTRY;

    enum GPU_OVERCLOCK_SETTING_AREA
    {
        GPU_OVERCLOCK_SETTING_AREA_CORE,
        GPU_OVERCLOCK_SETTING_AREA_MEMORY,
        GPU_OVERCLOCK_SETTING_AREA_SHADER,
        GPU_OVERCLOCK_SETTING_AREA_OVERVOLT
    };

    struct GpuOverclockSetting
    {
        bool editable;
        float currentValue;
        float minValue;
        float maxValue;
#ifdef __cplusplus
        GpuOverclockSetting();
        GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, const bool editable = false);
#endif
    };

    struct GpuOverclockProfile
    {
#ifdef __cplusplus
        const GpuOverclockSetting& operator[](const GPU_OVERCLOCK_SETTING_AREA area) const;
#endif
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
#ifdef __cplusplus
}
}
#endif