#pragma once
#include "helpers.h"
#include "nvidia_interface_datatypes.h"
#include <vector>
#include <memory>

class NvidiaGPU;

class NVLIB_EXPORTED NvidiaApi
{
public:
    NvidiaApi();
    ~NvidiaApi();
    int getGPUCount();
    std::shared_ptr<NvidiaGPU> getGPU(int index);
private:
    std::vector<std::shared_ptr<NvidiaGPU>> gpus;
    bool ensureGPUsLoaded();
    bool GPUloaded = false;
};

class NVLIB_EXPORTED NvidiaGPU
{
public:
    friend class NvidiaApi;
    float getCoreClock();
    float getMemoryClock();
    float getGPUUsage();
    float getFBUsage();
    float getVidUsage();
    float getBusUsage();
private:
    NV_PHYSICAL_GPU_HANDLE handle;
    NvidiaGPU(NV_PHYSICAL_GPU_HANDLE handle);
    struct NVIDIA_CLOCK_FREQUENCIES frequencies;
    bool reloadFrequencies();
    float getClockForSystem(NVIDIA_CLOCK_SYSTEM system);
    float getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM system);
};
