#pragma once

#include <memory>
#include "helpers.h"
#include "NvidiaApi.h"
#include "nvidia_interface_datatypes.h"

class NVLIB_EXPORTED NvidiaGPU
{
public:
    friend class NvidiaApi;
    bool poll();
    float getCoreClock();
    float getMemoryClock();
    float getGPUUsage();
    float getFBUsage();
    float getVidUsage();
    float getBusUsage();
private:
    NV_PHYSICAL_GPU_HANDLE handle;
    std::unique_ptr<NVIDIA_CLOCK_FREQUENCIES> frequencies;
    std::unique_ptr<NVIDIA_DYNAMIC_PSTATES> dynamicPstates;
    std::unique_ptr<NVIDIA_GPU_PSTATES20_V2> pstates20;
    std::unique_ptr<NVIDIA_GPU_POWER_POLICIES_INFO> powerPoliciesInfo;
    std::unique_ptr<NVIDIA_GPU_POWER_POLICIES_STATUS> powerPoliciesStatus;

    NvidiaGPU(NV_PHYSICAL_GPU_HANDLE handle);

    bool reloadFrequencies();
    float getClockForSystem(NVIDIA_CLOCK_SYSTEM system);
    float getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM system);
};