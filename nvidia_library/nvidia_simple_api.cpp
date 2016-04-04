#include "pch.h"
#include "nvidia_simple_api.h"
#include "NvidiaApi.h"

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
    }

    return usages;
}

NVLIB_EXPORTED bool init_simple_api()
{
    return ensureApi();
}
