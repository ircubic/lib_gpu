#pragma once

#include "pch.h"

#include <map>
#include <atomic>
#include "helpers.h"
#include "nvidia_interface_datatypes.h"

namespace lib_gpu {

enum GPU_OVERCLOCK_SETTING_AREA;
struct NvidiaGPUDataset;
struct GpuClocks;
struct GpuOverclockProfile;
struct GpuUsage;


#pragma warning(disable: 4251)
typedef std::map<GPU_OVERCLOCK_SETTING_AREA, float> GpuOverclockDefinitionMap;

class NVLIB_EXPORTED NvidiaGPU
{
public:
    NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle);
    ~NvidiaGPU();

    bool poll();

    std::string getName() const;
    std::string getSerialNumber() const;
    float getVoltage() const;
    float getTemperature() const;
    unsigned long getGPUID() const;

    std::unique_ptr<GpuClocks> getClocks() const;
    std::unique_ptr<GpuClocks> getDefaultClocks() const;
    std::unique_ptr<GpuClocks> getBaseClocks() const;
    std::unique_ptr<GpuClocks> getBoostClocks() const;
    std::unique_ptr<GpuOverclockProfile> getOverclockProfile() const;
    std::unique_ptr<GpuUsage> getUsage() const;

    bool setOverclock(const GpuOverclockDefinitionMap& overclockDefinitions);

private:
    const NV_PHYSICAL_GPU_HANDLE handle;
    const unsigned long GPUID;
    std::unique_ptr<NvidiaGPUDataset> dataset;

    std::unique_ptr<GpuClocks> getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE type, bool compensateForOverclock = false) const;
};
#pragma warning(default: 4251)

}