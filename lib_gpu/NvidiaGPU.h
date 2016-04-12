#pragma once

#include <memory>
#include <string>
#include <map>
#include "helpers.h"
#include "nvidia_interface_datatypes.h"
#include "GpuDatatypes.h"

namespace lib_gpu {
typedef std::map<GPU_OVERCLOCK_SETTING_AREA, float> GpuOverclockDefinitionMap;

struct NvidiaGPUDataset;

class NVLIB_EXPORTED NvidiaGPU
{
public:
    friend class NvidiaApi;
    ~NvidiaGPU();

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
    std::unique_ptr<NvidiaGPUDataset> dataset;

    NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle);
};
#pragma warning(default: 4251)

}