#include "pch.h"
#include "NvidiaApi.h"
#include "nvidia_interface.h"
#include <vector>
#include <algorithm>
#include "NvidiaGPU.h"

namespace lib_gpu {

NvidiaApi::NvidiaApi()
{
    if (!init_library()) {
        throw std::runtime_error("Unable to load NVIDIA API");
    }
}


NvidiaApi::~NvidiaApi()
{
}

std::vector<NV_PHYSICAL_GPU_HANDLE> load_gpu_handles()
{
    std::vector<NV_PHYSICAL_GPU_HANDLE> handles(0);
    const int MAX_HANDLES = 64;
    NV_PHYSICAL_GPU_HANDLE list[MAX_HANDLES];
    memset(list, 0, sizeof(NV_PHYSICAL_GPU_HANDLE) * MAX_HANDLES);
    unsigned long count = 0;

    if (NVIDIA_RAW_GetPhysicalGPUHandles(list, &count) == NVAPI_OK) {
        for (size_t i = 0; i < count; i++) {
            handles.push_back(list[i]);
        }
    }

    return handles;
}

unsigned int NvidiaApi::getGPUCount() const
{
    if (this->ensureGPUsLoaded()) {
        return this->gpus.size();
    }
    return 0;
}

std::shared_ptr<NvidiaGPU> NvidiaApi::getGPU(const unsigned int index) const
{
    this->ensureGPUsLoaded();
    return index < this->gpus.size() ? this->gpus[index] : nullptr;
}

bool NvidiaApi::ensureGPUsLoaded() const
{
    if (!this->GPUloaded) {
        std::vector<NV_PHYSICAL_GPU_HANDLE> handles = load_gpu_handles();
        if (handles.size() > 0) {
            this->gpus.clear();
            for each (NV_PHYSICAL_GPU_HANDLE handle in handles)
            {
                auto gpu = std::make_shared<NvidiaGPU>(handle);
                this->gpus.push_back(gpu);
            }

            this->GPUloaded = true;
        } 
    }
    return this->GPUloaded;
}

}