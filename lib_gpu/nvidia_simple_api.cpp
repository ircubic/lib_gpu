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

std::shared_ptr<NvidiaGPU> getUpdatedGPU(int num = 0) {
    if (ensureApi()) {
        auto gpu = api->getGPU(num);
        if (gpu) {
            gpu->poll();
            return gpu;
        }
    }

    return nullptr;
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

struct GpuUsage get_usages()
{
    auto gpu = getUpdatedGPU();
    if (gpu) {
        auto usage = gpu->getUsage();
        if (usage) {
            return *usage;
        }
    }

    return {};
}

struct GpuOverclockProfile get_overclock_profile()
{
    if (auto gpu = getUpdatedGPU()) {
        return *gpu->getOverclockProfile();
    }

    return {};
}

NVLIB_EXPORTED bool overclock(float new_delta, int area)
{
    auto gpu = getUpdatedGPU();
    bool success = false;
    if (gpu) {
        GpuOverclockDefinitionMap map;
        map[(GPU_OVERCLOCK_SETTING_AREA)area] = new_delta;
        success = gpu->setOverclock(map);
    }
    return success;
}

NVLIB_EXPORTED bool init_simple_api()
{
    return ensureApi();
}
