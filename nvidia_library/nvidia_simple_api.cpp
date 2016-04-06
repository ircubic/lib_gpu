#include "pch.h"
#include "nvidia_simple_api.h"
#include "NvidiaApi.h"
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


int get_best_pstate_index(struct NVIDIA_GPU_PSTATES20_V1& pstates) {
    int best_pstate_index = 0;
    int best_pstate_state = 0;
    for (size_t i = 0; i < pstates.state_count; i++) {
        if (pstates.states[i].state_num < best_pstate_state) {
            best_pstate_index = i;
        }
    }
    return best_pstate_index;
}

struct nvidia_simple_overclock_profile get_overclock_profile()
{
    struct nvidia_simple_overclock_profile profile = {0};

    if (ensureApi()) {
        NVIDIA_GPU_PSTATES20_V1 pstates;
        REINIT_NVIDIA_STRUCT(pstates);
        NV_PHYSICAL_GPU_HANDLE handles[64];
        unsigned long count = 0;
        if (NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count) == NVAPI_OK &&
            NVIDIA_RAW_GetPstates20(handles[0], &pstates) == NVAPI_OK) {
            int best_pstate_index = get_best_pstate_index(pstates);
            auto best_pstate = pstates.states[best_pstate_index];
            int core_clock_index = 0;
            for (size_t i = 0; i < pstates.clock_count; i++)
            {
                if (best_pstate.clocks[i].domain == NVIDIA_CLOCK_SYSTEM_GPU) {
                    core_clock_index = i;
                }
            }
            auto core_clock = best_pstate.clocks[core_clock_index];
            profile.coreOverclock.currentValue = core_clock.freq_delta.value / 1000.0;
            profile.coreOverclock.maxValue = core_clock.freq_delta.val_max / 1000.0;
            profile.coreOverclock.minValue = core_clock.freq_delta.val_min / 1000.0;
        }
    }

    return profile;
}

NVLIB_EXPORTED bool overclock(float new_delta, int clock)
{
    bool result = false;
    NVIDIA_CLOCK_SYSTEM system = (NVIDIA_CLOCK_SYSTEM)clock;
    struct nvidia_simple_overclock_profile profile = get_overclock_profile();
    if (clock == NVIDIA_CLOCK_SYSTEM_GPU && new_delta >= profile.coreOverclock.minValue && new_delta <= profile.coreOverclock.maxValue) {
        NVIDIA_GPU_PSTATES20_V1 pstates;
        REINIT_NVIDIA_STRUCT(pstates);
        NV_PHYSICAL_GPU_HANDLE handles[64];
        unsigned long count = 0;
        if (NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count) == NVAPI_OK &&
            NVIDIA_RAW_GetPstates20(handles[0], &pstates) == NVAPI_OK) {
            int best_pstate_index = get_best_pstate_index(pstates);
            int best_pstate = pstates.states[best_pstate_index].state_num;
            REINIT_NVIDIA_STRUCT(pstates);
            pstates.clock_count = 1;
            pstates.state_count = 1;
            pstates.states[0].state_num = best_pstate;
            auto overclockClock = &pstates.states[0].clocks[0];
            overclockClock->domain = clock;
            overclockClock->freq_delta.value = (INT32)(new_delta * 1000);
            result = NVIDIA_RAW_SetPstates20(handles[0], &pstates) == NVAPI_OK;
        }
    }
    return result;
}

NVLIB_EXPORTED bool init_simple_api()
{
    return ensureApi();
}
