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

    return api != nullptr;
}

std::shared_ptr<NvidiaGPU> getUpdatedGPU(unsigned num = 0)
{
    if (ensureApi()) {
        std::lock_guard<std::mutex> lock(api_mutex);
        const auto gpu = api->getGPU(num);

        if (gpu) {
            const auto now = GetTickCount64();
            const auto last = last_poll[num];
            auto poll_success = true;

            if (now - last > MIN_POLL_INTERVAL) {
                poll_success = gpu->poll();
                last_poll[num] = now;
            }

            if (poll_success) {
                return gpu;
            }
        }
    }

    return nullptr;
}

unsigned get_gpu_count()
{
    return ensureApi() ? api->getGPUCount() : 0;
}

unsigned get_index_for_GPUID(unsigned long GPUID)
{
    return ensureApi() ? api->getIndexForGPUID(GPUID) : 0;
}

template <typename T, typename F>
T fetch_with_gpu(unsigned gpu_index, F fetcher)
{
    const auto gpu = getUpdatedGPU(gpu_index);

    return gpu ? fetcher(gpu) : T{};
}

template <typename T, typename P = std::unique_ptr<T>>
T fetch_with_gpu(unsigned gpu_index, P(*fetcher)(std::shared_ptr<NvidiaGPU>))
{
    P ptr = fetch_with_gpu<P, decltype(fetcher)>(gpu_index, fetcher);

    return ptr ? *ptr : T{};
}


unsigned long getGPUID(unsigned gpu_index)
{
    return fetch_with_gpu<unsigned long>(gpu_index, [](auto gpu) {
        return gpu->getGPUID();
    });
}

float get_voltage(unsigned gpu_index)
{
    return fetch_with_gpu<float>(gpu_index, [](auto gpu) {
        return gpu->getVoltage();
    });
}

float get_temperature(unsigned gpu_index)
{
    return fetch_with_gpu<float>(gpu_index, [](auto gpu) {
        return gpu->getTemperature();
    });
}

struct GpuClocks get_clocks(unsigned gpu_index)
{
    return fetch_with_gpu<GpuClocks, std::unique_ptr<GpuClocks>>(gpu_index, [](auto gpu) {
        return gpu->getClocks();
    });
}

struct GpuClocks get_default_clocks(unsigned gpu_index)
{
    return fetch_with_gpu<GpuClocks, std::unique_ptr<GpuClocks>>(gpu_index, [](auto gpu) {
        return gpu->getDefaultClocks();
    });
}

struct GpuClocks get_base_clocks(unsigned gpu_index)
{
    return fetch_with_gpu<GpuClocks, std::unique_ptr<GpuClocks>>(gpu_index, [](auto gpu) {
        return gpu->getBaseClocks();
    });
}

struct GpuClocks get_boost_clocks(unsigned gpu_index)
{
    return fetch_with_gpu<GpuClocks, std::unique_ptr<GpuClocks>>(gpu_index, [](auto gpu) {
        return gpu->getBoostClocks();
    });
}

struct GpuUsage get_usages(unsigned gpu_index)
{
    return fetch_with_gpu<GpuUsage, std::unique_ptr<GpuUsage>>(gpu_index, [](auto gpu) {
        return gpu->getUsage();
    });
}

struct GpuOverclockProfile get_overclock_profile(unsigned gpu_index)
{
    return fetch_with_gpu<GpuOverclockProfile, std::unique_ptr<GpuOverclockProfile>>(gpu_index, [](auto gpu) {
        return gpu->getOverclockProfile();
    });
}

bool overclock(unsigned gpu_index, unsigned area, float new_delta)
{
    return fetch_with_gpu<bool>(gpu_index, [&](auto gpu) -> bool {
        GpuOverclockDefinitionMap map;
        map[static_cast<GPU_OVERCLOCK_SETTING_AREA>(area)] = new_delta;
        bool success = gpu->setOverclock(map);

        return success;
    });
}

bool init_simple_api()
{
    return ensureApi();
}

bool get_name(unsigned gpu_index, char name[NVIDIA_SHORT_STRING_SIZE])
{
    if (name) {
        return fetch_with_gpu<bool>(gpu_index, [&](auto gpu) {
            std::string str = gpu->getName();
            size_t copied = str._Copy_s(name, NVIDIA_SHORT_STRING_SIZE - 1, str.size());
            name[copied] = '\0';
            return copied > 0;
        });
    }
    return false;
}

bool get_serial_number(unsigned gpu_index, char serial[NVIDIA_SHORT_STRING_SIZE])
{
    if (serial) {
        return fetch_with_gpu<bool>(gpu_index, [&](auto gpu) {
            std::string str = gpu->getSerialNumber();
            size_t copied = str._Copy_s(serial, NVIDIA_SHORT_STRING_SIZE - 1, str.size());
            serial[copied] = '\0';
            return copied > 0;
        });
    }
    return false;
}

}
}