#include "pch.h"
#include "nvidia_simple_api.h"
#include "lib_gpu_nvidia.h"
#include "nvidia_interface.h"
#include <mutex>

namespace lib_gpu {
namespace nvidia_simple_api {

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

std::shared_ptr<NvidiaGPU> getUpdatedGPU(unsigned int num = 0)
{
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

unsigned int get_gpu_count()
{
    if (ensureApi()) {
        return api->getGPUCount();
    }

    return 0;
}

template <typename T, typename F>
T fetch_with_gpu(unsigned int gpu_index, F fetcher)
{
    auto gpu = getUpdatedGPU(gpu_index);
    if (gpu) {
        return fetcher(gpu);
    }

    return{};
}

template <typename T, typename P>
T fetch_with_gpu(unsigned int gpu_index, P(*fetcher)(std::shared_ptr<NvidiaGPU>))
{
    P ptr = fetch_with_gpu<P, decltype(fetcher)>(gpu_index, fetcher);
    if (ptr) {
        T real = *ptr;
        return real;
    } else {
        return{};
    }
}


struct GpuClocks get_clocks(unsigned int gpu_index)
{
    return fetch_with_gpu<GpuClocks, std::unique_ptr<GpuClocks>>(gpu_index, [](std::shared_ptr<NvidiaGPU> gpu) -> std::unique_ptr<GpuClocks> {
        return gpu->getClocks();
    });
}

struct GpuUsage get_usages(unsigned int gpu_index)
{
    return fetch_with_gpu<GpuUsage, std::unique_ptr<GpuUsage>>(gpu_index, [](std::shared_ptr<NvidiaGPU> gpu) -> std::unique_ptr<GpuUsage> {
        return gpu->getUsage();
    });
}

struct GpuOverclockProfile get_overclock_profile(unsigned int gpu_index)
{
    return fetch_with_gpu<GpuOverclockProfile, std::unique_ptr<GpuOverclockProfile>>(gpu_index, [](std::shared_ptr<NvidiaGPU> gpu) -> std::unique_ptr<GpuOverclockProfile> {
        return gpu->getOverclockProfile();
    });
}

bool overclock(unsigned int gpu_index, unsigned int area, float new_delta)
{
    return fetch_with_gpu<bool>(gpu_index, [&](std::shared_ptr<NvidiaGPU> gpu) -> bool {
        GpuOverclockDefinitionMap map;
        map[(GPU_OVERCLOCK_SETTING_AREA)area] = new_delta;
        bool success = gpu->setOverclock(map);

        // We force a poll just to ensure the overclock profile is correct
        gpu->poll();
        return success;
    });
}

bool init_simple_api()
{
    return ensureApi();
}

bool get_name(unsigned int gpu_index, char name[NVIDIA_SHORT_STRING_SIZE])
{
    if (name) {
        return fetch_with_gpu<bool>(gpu_index, [&](std::shared_ptr<NvidiaGPU> gpu) -> bool {
            std::string str = gpu->getName();
            size_t copied = str._Copy_s(name, NVIDIA_SHORT_STRING_SIZE - 1, str.size());
            name[copied] = '\0';
            return copied > 0;
        });
    }
    return false;
}

}
}