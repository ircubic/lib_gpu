#pragma once
#include <vector>
#include <memory>

#include "helpers.h"
#include "NvidiaGpu.h"

namespace lib_gpu {

class NVLIB_EXPORTED NvidiaApi
{
public:
    NvidiaApi();
    ~NvidiaApi();
    unsigned getGPUCount() const;
    unsigned getIndexForGPUID(unsigned long GPUID) const;
    std::shared_ptr<NvidiaGPU> getGPU(unsigned index) const;
private:
#pragma warning(disable: 4251)
    mutable std::vector<std::shared_ptr<NvidiaGPU>> gpus;
#pragma warning(default: 4251)
    bool ensureGPUsLoaded() const;
    mutable bool GPUloaded = false;
};

}