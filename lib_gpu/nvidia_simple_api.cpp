#include "pch.h"
#include "nvidia_simple_api.h"
#include "lib_gpu_nvidia.h"
#include "nvidia_interface.h"

static std::unique_ptr<NvidiaApi> api{};
static ULONGLONG last_poll = 0;

// To avoid unnecessary polling with the simple API, we enforce a max pollrate
const int MAX_POLLS_PER_SEC = 4;
const ULONGLONG MIN_POLL_INTERVAL = 1000 / MAX_POLLS_PER_SEC;

bool ensureApi() 
{
    if (!api) {
        api.reset(new NvidiaApi());
        last_poll = 0;
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
            ULONGLONG now = GetTickCount64();

            if (now - last_poll > MIN_POLL_INTERVAL) {
                gpu->poll();
                last_poll = now;
            }

            return gpu;
        }
    }

    return nullptr;
}

struct GpuClocks get_clocks()
{
    auto gpu = getUpdatedGPU();
    if (gpu) {
        auto clocks = gpu->getClocks();
        if (clocks) {
            return *clocks;
        }
    }

    return {};
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

bool overclock(float new_delta, int area)
{
    auto gpu = getUpdatedGPU();
    bool success = false;
    if (gpu) {
        GpuOverclockDefinitionMap map;
        map[(GPU_OVERCLOCK_SETTING_AREA)area] = new_delta;
        success = gpu->setOverclock(map);

        // We force a poll just to ensure the overclock profile is correct
        gpu->poll();
    }
    return success;
}

bool init_simple_api()
{
    return ensureApi();
}
