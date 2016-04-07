#include "pch.h"
#include "NvidiaApi.h"
#include "nvidia_interface.h"
#include <vector>
#include <algorithm>

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

float NvidiaGPU::getClockForSystem(NVIDIA_CLOCK_SYSTEM system)
{
    if (this->reloadFrequencies()) {
        if (this->frequencies.entries[system].present) {
            return this->frequencies.entries[system].freq / 1000.0;
        }
    }

    return -1;
}

float NvidiaGPU::getCoreClock()
{
    return this->getClockForSystem(NVIDIA_CLOCK_SYSTEM_GPU);    
}

float NvidiaGPU::getMemoryClock()
{

    return this->getClockForSystem(NVIDIA_CLOCK_SYSTEM_MEMORY);
}

float NvidiaGPU::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM system)
{
    struct NVIDIA_DYNAMIC_PSTATES pstates;
    REINIT_NVIDIA_STRUCT(pstates);
    if (NVIDIA_RAW_GetDynamicPStates(this->handle, &pstates) == NVAPI_OK) {
        auto state = pstates.pstates[system];
        if (state.present) {
            return state.value;
        }
    }

    return -1;
}

float NvidiaGPU::getGPUUsage()
{
    return this->getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_GPU);
}

float NvidiaGPU::getFBUsage()
{
    return this->getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_FB);
}

float NvidiaGPU::getVidUsage()
{
    return this->getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_VID);
}

float NvidiaGPU::getBusUsage()
{
    return this->getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_BUS);
}

NvidiaGPU::NvidiaGPU(NV_PHYSICAL_GPU_HANDLE handle)
{
    this->handle = handle;
}
bool NvidiaGPU::reloadFrequencies()
{
    memset(this->frequencies.entries, 0, sizeof(this->frequencies.entries));
    this->frequencies.clock_type = 0; // queries current frequency
    return NVIDIA_RAW_GetAllClockFrequencies(this->handle, &this->frequencies) == NVAPI_OK;
}
