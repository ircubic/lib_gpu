#pragma once

#include <memory>
#include <string>
#include <map>
#include "helpers.h"
#include "nvidia_interface_datatypes.h"
#include "GpuDatatypes.h"

typedef std::map<GPU_OVERCLOCK_SETTING_AREA, float> GpuOverclockDefinitionMap;

class NVLIB_EXPORTED NvidiaGPU
{
public:
    friend class NvidiaApi;
    bool poll();

    std::string getName();
    float getVoltage();
    float getTemp();

#pragma warning(disable: 4251)
    std::unique_ptr<GpuClocks> getClocks();
    std::unique_ptr<GpuOverclockProfile> getOverclockProfile();
    std::unique_ptr<GpuUsage> getUsage();

    bool setOverclock(const GpuOverclockDefinitionMap& overclockDefinitions);

private:
    NV_PHYSICAL_GPU_HANDLE handle;
    std::unique_ptr<NVIDIA_CLOCK_FREQUENCIES> frequencies;
    std::unique_ptr<NVIDIA_DYNAMIC_PSTATES> dynamicPstates;
    std::unique_ptr<NVIDIA_GPU_PSTATES20_V2> pstates20;
    std::unique_ptr<NVIDIA_GPU_POWER_POLICIES_INFO> powerPoliciesInfo;
    std::unique_ptr<NVIDIA_GPU_POWER_POLICIES_STATUS> powerPoliciesStatus;
    std::unique_ptr<NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS> voltageDomainsStatus;
    std::unique_ptr<NVIDIA_GPU_THERMAL_SETTINGS_V2> thermalSettings;

    NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle);
};
#pragma warning(default: 4251)

