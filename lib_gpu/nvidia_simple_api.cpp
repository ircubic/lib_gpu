#include "pch.h"
#include "nvidia_simple_api.h"
#include "lib_gpu_nvidia.h"
#include "nvidia_interface.h"
#include <mutex>

static std::unique_ptr<NvidiaApi> api{};
static std::vector<ULONGLONG> last_poll;
static std::mutex api_mutex;

// To avoid unnecessary polling with the simple API, we enforce a max pollrate
const int MAX_POLLS_PER_SEC = 4;
const ULONGLONG MIN_POLL_INTERVAL = 1000 / MAX_POLLS_PER_SEC;

bool ensureApi() 
{
    std::lock_guard<std::mutex> lock(api_mutex);
    if (!api) {
        api.reset(new NvidiaApi());
        last_poll.clear();
        auto count = api->getGPUCount();
        if (count <= 0) {
            api.reset(nullptr);
        } else {
            last_poll.resize(count, 0);
        }
    }
    return (bool)api;
}

std::shared_ptr<NvidiaGPU> getUpdatedGPU(unsigned int num = 0) {
    if (ensureApi()) {
        std::lock_guard<std::mutex> lock(api_mutex);
        auto gpu = api->getGPU(num);
        if (gpu) {
            ULONGLONG now = GetTickCount64();
            auto last = last_poll[num];
            if (now - last > MIN_POLL_INTERVAL) {
                gpu->poll();
                last_poll[num] = now;
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
