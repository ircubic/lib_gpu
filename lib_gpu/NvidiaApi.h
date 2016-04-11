#pragma once
#include <vector>
#include <memory>

#include "helpers.h"
#include "NvidiaGpu.h"

namespace lib_gpu
{

class NVLIB_EXPORTED NvidiaApi
{
public:
    NvidiaApi();
    ~NvidiaApi();
    unsigned int getGPUCount();
    std::shared_ptr<NvidiaGPU> getGPU(unsigned int index);
private:
#pragma warning(disable: 4251)
    std::vector<std::shared_ptr<NvidiaGPU>> gpus;
#pragma warning(default: 4251)
    bool ensureGPUsLoaded();
    bool GPUloaded = false;
};

}