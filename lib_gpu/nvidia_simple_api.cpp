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


int get_best_pstate_index(struct NVIDIA_GPU_PSTATES20_V2& pstates) {
    int best_pstate_index = 0;
    int best_pstate_state = 0;
    for (size_t i = 0; i < pstates.state_count; i++) {
        if (pstates.states[i].state_num < best_pstate_state) {
            best_pstate_index = i;
        }
    }
    return best_pstate_index;
}
#define TO_MHZ(x) (x / 1000.0)

struct nvidia_simple_overclock_setting get_setting_from_pstate(NVIDIA_GPU_PSTATES20_V2& pstates, int state_index, int clock_index)
{
    auto pstate = pstates.states[state_index];
    auto freq_delta = pstate.clocks[clock_index].freq_delta;
    return {
        TO_MHZ(freq_delta.value),
        TO_MHZ(freq_delta.val_min),
        TO_MHZ(freq_delta.val_max)
    };
}

struct nvidia_simple_overclock_profile get_overclock_profile()
{
    struct nvidia_simple_overclock_profile profile = {0};

    if (ensureApi()) {
        NVIDIA_GPU_PSTATES20_V2 pstates;
        REINIT_NVIDIA_STRUCT(pstates);
        NV_PHYSICAL_GPU_HANDLE handles[64];
        unsigned long count = 0;
        if (NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count) == NVAPI_OK &&
            NVIDIA_RAW_GetPstates20(handles[0], &pstates) == NVAPI_OK) {
            int best_pstate_index = get_best_pstate_index(pstates);
            auto best_pstate = pstates.states[best_pstate_index];
            int core_clock_index = INT_MAX;
            int memory_clock_index = INT_MAX;
            for (size_t i = 0; i < pstates.clock_count; i++)
            {
                switch (best_pstate.clocks[i].domain) {
                case NVIDIA_CLOCK_SYSTEM_GPU:
                    core_clock_index = i;
                    break;
                case NVIDIA_CLOCK_SYSTEM_MEMORY:
                    memory_clock_index = i;
                    break;
                }

            }
            if (core_clock_index < pstates.clock_count) {
                profile.coreOverclock = get_setting_from_pstate(pstates, best_pstate_index, core_clock_index);
            }
            if (memory_clock_index < pstates.clock_count) {
                profile.memoryOverclock = get_setting_from_pstate(pstates, best_pstate_index, memory_clock_index);
            }
        }
    }

    return profile;
}

NVLIB_EXPORTED bool overclock(float new_delta, int clock)
{
    bool result = false;
    NVIDIA_CLOCK_SYSTEM system = (NVIDIA_CLOCK_SYSTEM)clock;
    auto profile = get_overclock_profile();

    auto setting = nvidia_simple_overclock_setting { 0.0 };
    switch (clock) {
    case NVIDIA_CLOCK_SYSTEM_GPU:
        setting = profile.coreOverclock;
        break;
    case NVIDIA_CLOCK_SYSTEM_MEMORY:
        setting = profile.memoryOverclock;
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
            int best_pstate_index = get_best_pstate_index(pstates);
            int best_pstate = pstates.states[best_pstate_index].state_num;
            REINIT_NVIDIA_STRUCT(pstates);
            pstates.clock_count = 1;
            pstates.state_count = 1;
            pstates.states[0].state_num = best_pstate;
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
