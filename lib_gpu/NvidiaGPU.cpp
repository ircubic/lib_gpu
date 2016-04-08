#include "pch.h"
#include "NvidiaGPU.h"
#include "nvidia_interface.h"

template<typename T>
std::unique_ptr<T> loadNvidiaStruct(NV_PHYSICAL_GPU_HANDLE const& handle, NV_STATUS (*loader)(NV_PHYSICAL_GPU_HANDLE, T*), void (*preparer)(T*) = nullptr) {
    auto struct_ = std::make_unique<T>();
    UINT32 version = struct_->version;
    memset(struct_.get(), 0, sizeof(T));
    struct_->version = version;
    if (preparer) {
        preparer(struct_.get());
    }
    return (*loader)(handle, struct_.get()) == NVAPI_OK ? std::move(struct_) : nullptr;
}

std::unique_ptr<NVIDIA_CLOCK_FREQUENCIES> loadCLOCK_FREQUENCIES(NV_PHYSICAL_GPU_HANDLE const& handle)
{
    return loadNvidiaStruct<NVIDIA_CLOCK_FREQUENCIES>(handle, NVIDIA_RAW_GetAllClockFrequencies, [](NVIDIA_CLOCK_FREQUENCIES* f) {f->clock_type = 0; });
}

#define SIMPLE_NVIDIA_CALL(T_, function_) std::unique_ptr<NVIDIA_##T_> load ## T_(NV_PHYSICAL_GPU_HANDLE const& handle) { return loadNvidiaStruct<NVIDIA_##T_>(handle, function_); }

SIMPLE_NVIDIA_CALL(DYNAMIC_PSTATES, NVIDIA_RAW_GetDynamicPStates);
SIMPLE_NVIDIA_CALL(GPU_PSTATES20_V2, NVIDIA_RAW_GetPstates20);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_INFO, NVIDIA_RAW_GpuClientPowerPoliciesGetInfo);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_STATUS, NVIDIA_RAW_GpuClientPowerPoliciesGetStatus);


float NvidiaGPU::getClockForSystem(NVIDIA_CLOCK_SYSTEM system)
{
    if (this->reloadFrequencies()) {
        if (this->frequencies->entries[system].present) {
            return this->frequencies->entries[system].freq / 1000.0;
        }
    }

    return -1;
}

bool NvidiaGPU::poll()
{
    auto frequencies = loadCLOCK_FREQUENCIES(this->handle);
    auto dynamicPstates = loadDYNAMIC_PSTATES(this->handle);
    auto pstates20 = loadGPU_PSTATES20_V2(this->handle);
    //auto perfTable = std::make_shared<NVIDIA_GPU_PERF_TABLE>();
    auto powerPoliciesInfo = loadGPU_POWER_POLICIES_INFO(this->handle);
    auto powerPoliciesStatus = loadGPU_POWER_POLICIES_STATUS(this->handle);

    auto success = false;

    if (frequencies && dynamicPstates && pstates20 && powerPoliciesInfo && powerPoliciesStatus) {
        success = true;
        this->frequencies = std::move(frequencies);
        this->dynamicPstates = std::move(dynamicPstates);
        this->pstates20 = std::move(pstates20);
        this->powerPoliciesInfo = std::move(powerPoliciesInfo);
        this->powerPoliciesStatus = std::move(powerPoliciesStatus);
    }

    return success;
}

float NvidiaGPU::getCoreClock()
{
    return this->getClockForSystem(NVIDIA_CLOCK_SYSTEM_GPU);
}

float NvidiaGPU::getMemoryClock()
{

    return this->getClockForSystem(NVIDIA_CLOCK_SYSTEM_MEMORY);
}

#define LIFT_UNIT(x) (x/1000.0)

int get_best_pstate_index(NVIDIA_GPU_PSTATES20_V2 const& pstates) {
    int best_pstate_index = 0;
    int best_pstate_state = INT_MAX;
    for (size_t i = 0; i < pstates.state_count; i++) {
        if (pstates.states[i].state_num < best_pstate_state) {
            best_pstate_index = i;
            best_pstate_state = pstates.states[i].state_num;
        }
    }
    return best_pstate_index;
}

std::unique_ptr<GpuOverclockProfile> NvidiaGPU::getOverclockProfile()
{
    auto profile = std::make_unique<GpuOverclockProfile>();
    int best_pstate_index = get_best_pstate_index(*this->pstates20);
    auto best_pstate = this->pstates20->states[best_pstate_index];
    auto fetcher = [&](int i) { return GpuOverclockSetting<MHz>(best_pstate.clocks[i].freq_delta, (best_pstate.flags & 1)); };

    for (int i = 0; i < this->pstates20->clock_count; i++)
    {
        switch (best_pstate.clocks[i].domain) {
        case NVIDIA_CLOCK_SYSTEM_GPU:
            profile->coreOverclock = fetcher(i);
            break;
        case NVIDIA_CLOCK_SYSTEM_MEMORY:
            profile->memoryOverclock = fetcher(i);
            break;
        case NVIDIA_CLOCK_SYSTEM_SHADER:
            profile->shaderOverclock = fetcher(i);
            break;
        }

    }

    return profile;
}

float getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM system, const NVIDIA_DYNAMIC_PSTATES& pstates)
{
    auto state = pstates.pstates[system];
    return state.present ? state.value : -1.0;
}

std::unique_ptr<GpuUsage> NvidiaGPU::getUsage()
{
    if (this->dynamicPstates) {
        return std::unique_ptr<GpuUsage>(new GpuUsage{
            ::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_GPU, *this->dynamicPstates),
            ::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_FB, *this->dynamicPstates),
            ::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_VID, *this->dynamicPstates),
            ::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_BUS, *this->dynamicPstates)
        });
    }
    return nullptr;
}

float NvidiaGPU::getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM system)
{
    if(this->poll()) {
        return ::getUsageForSystem(system, *this->dynamicPstates);
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

NvidiaGPU::NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle)
{
    this->handle = handle;
}

bool NvidiaGPU::reloadFrequencies()
{
    auto frequencies = loadCLOCK_FREQUENCIES(this->handle);
    bool success = false;
    if (frequencies) {
        this->frequencies = std::move(frequencies);
        success = true;
    }
    return success;
}

template<typename T>
GpuOverclockSetting<T>::GpuOverclockSetting()
{
    this->editable = false;
    this->currentValue = this->maxValue = this->minValue = 0.0;
}

template<typename T>
GpuOverclockSetting<T>::GpuOverclockSetting(NVIDIA_DELTA_ENTRY const& delta, bool editable)
{
    this->editable = editable;
    this->currentValue = LIFT_UNIT(delta.value);
    this->minValue = LIFT_UNIT(delta.val_min);
    this->maxValue = LIFT_UNIT(delta.val_max);
}