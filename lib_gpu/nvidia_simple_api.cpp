#include "pch.h"
#include "nvidia_simple_api.h"
#include "lib_gpu_nvidia.h"
#include "nvidia_interface.h"

static std::unique_ptr<NvidiaApi> api{};

bool ensureApi() 
{
    if (!api) {
        api.reset(new NvidiaApi());
        if (api->getGPUCount() <= 0) {
            api.reset(nullptr);
        }
    }
    return (bool)api;
}

void init_simple_clocks(struct nvidia_simple_clocks& clocks) {
    clocks.coreClock = -1;
    clocks.memoryClock = -1;
    clocks.shaderClock = -1;
}

struct nvidia_simple_clocks get_clocks()
{
    struct nvidia_simple_clocks clocks;
    init_simple_clocks(clocks);
    if (ensureApi()) {
        auto gpu = api->getGPU(0);
        clocks.coreClock = gpu->getCoreClock();
        clocks.memoryClock = gpu->getMemoryClock();
    }
    return clocks;
}

void init_simple_usages(struct nvidia_simple_usages& usages) {
    usages.gpuUsage = -1;
    usages.busUsage = -1;
    usages.fbUsage = -1;
    usages.vidUsage = -1;
}

struct nvidia_simple_usages get_usages()
{
    struct nvidia_simple_usages usages;
    init_simple_usages(usages);

    if (ensureApi()) {
        auto gpu = api->getGPU(0);
        usages.gpuUsage = gpu->getGPUUsage();
        usages.fbUsage = gpu->getFBUsage();
        usages.vidUsage = gpu->getVidUsage();
        usages.busUsage = gpu->getBusUsage();
    }

    return usages;
}

struct GpuOverclockProfile get_overclock_profile()
{
    if (ensureApi()) {
        auto gpu = api->getGPU(0);
        gpu->poll();
        return *gpu->getOverclockProfile();
    }

    return{};
}

NVLIB_EXPORTED bool overclock(float new_delta, int clock)
{
    auto convert_setting = [](GpuOverclockSetting const& setting)-> nvidia_simple_overclock_setting {
        return{ setting.currentValue, setting.minValue, setting.maxValue };
    };
    bool result = false;
    NVIDIA_CLOCK_SYSTEM system = (NVIDIA_CLOCK_SYSTEM)clock;
    auto profile = get_overclock_profile();

    auto setting = nvidia_simple_overclock_setting { 0.0 };
    switch (clock) {
    case NVIDIA_CLOCK_SYSTEM_GPU:
        setting = convert_setting(profile.coreOverclock);
        break;
    case NVIDIA_CLOCK_SYSTEM_MEMORY:
        setting = convert_setting(profile.memoryOverclock);
        break;
    default:
        return false;
    }

    INT32 new_value = (INT32)(new_delta * 1000);
    if (new_value == new_delta) {
        result = true;
    } else if (new_delta >= setting.minValue && new_delta <= setting.maxValue) {
        NVIDIA_GPU_PSTATES20_V2 pstates;
        REINIT_NVIDIA_STRUCT(pstates);
        NV_PHYSICAL_GPU_HANDLE handles[64];
        unsigned long count = 0;

        if (NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count) == NVAPI_OK &&
            NVIDIA_RAW_GetPstates20(handles[0], &pstates) == NVAPI_OK) {
            REINIT_NVIDIA_STRUCT(pstates);
            pstates.clock_count = 1;
            pstates.state_count = 1;
            pstates.states[0].state_num = 0;
            auto overclockClock = &(pstates.states[0].clocks[0]);
            overclockClock->domain = clock;
            overclockClock->freq_delta.value = new_value;
            overclockClock->type = 0;
            result = NVIDIA_RAW_SetPstates20(handles[0], &pstates) == NVAPI_OK;
        }
    }
    return result;
}

NVLIB_EXPORTED bool init_simple_api()
{
    return ensureApi();
}
