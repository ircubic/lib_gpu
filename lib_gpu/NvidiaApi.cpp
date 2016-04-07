#include "pch.h"
#include "NvidiaApi.h"
#include "nvidia_interface.h"
#include <vector>
#include <algorithm>
#include "NvidiaGPU.h"

NvidiaApi::NvidiaApi()
{
    int ret = init_library();
    if (!ret || NVIDIA_RAW_NvidiaInit()) {
        throw "OH NO";
    }
}


NvidiaApi::~NvidiaApi()
{
    NVIDIA_RAW_NvidiaUnload();
}

std::vector<NV_PHYSICAL_GPU_HANDLE> load_gpu_handles() {
    std::vector<NV_PHYSICAL_GPU_HANDLE> handles(0);
    const int MAX_HANDLES = 64;
    NV_PHYSICAL_GPU_HANDLE list[MAX_HANDLES];
    memset(list, 0, sizeof(NV_PHYSICAL_GPU_HANDLE) * MAX_HANDLES);
    unsigned long count = 0;

    if (NVIDIA_RAW_GetPhysicalGPUHandles(list, &count) == NVAPI_OK) {
        for (size_t i = 0; i < count; i++)
        {
            handles.push_back(list[i]);
        }
    }

    return handles;
}

int NvidiaApi::getGPUCount()
{
    if (this->ensureGPUsLoaded()) {
        return this->gpus.size();
    }
    return 0;
}

std::shared_ptr<NvidiaGPU> NvidiaApi::getGPU(int index)
{
    this->ensureGPUsLoaded();
    return this->gpus.at(index);
}

bool NvidiaApi::ensureGPUsLoaded()
{
    if (!this->GPUloaded) {
        std::vector<NV_PHYSICAL_GPU_HANDLE> handles = load_gpu_handles();
        if (handles.size() > 0) {
            this->gpus.clear();
            for each (NV_PHYSICAL_GPU_HANDLE handle in handles)
            {
                auto gpu = std::shared_ptr<NvidiaGPU>(new NvidiaGPU(handle));
                this->gpus.push_back(gpu);
            }
            this->gpus.shrink_to_fit();
            this->GPUloaded = true;
            return true;
        }
    }
    this->GPUloaded = false;
    return false;
}
