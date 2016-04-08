#pragma once
#include <vector>
#include <memory>

#include "helpers.h"
#include "NvidiaGpu.h"

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
