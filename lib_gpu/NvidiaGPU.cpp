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

std::unique_ptr<NVIDIA_GPU_THERMAL_SETTINGS_V2> loadGPU_THERMAL_SETTINGS_V2(NV_PHYSICAL_GPU_HANDLE const& handle)
{
    auto loader_wrapper = [](NV_PHYSICAL_GPU_HANDLE handle, NVIDIA_GPU_THERMAL_SETTINGS_V2* settings) {
        return NVIDIA_RAW_GpuGetThermalSettings(handle, NVIDIA_THERMAL_TARGET_ALL, settings);
    };

    return loadNvidiaStruct<NVIDIA_GPU_THERMAL_SETTINGS_V2>(handle, loader_wrapper);
}

#define SIMPLE_NVIDIA_CALL(T_, function_) std::unique_ptr<NVIDIA_##T_> load ## T_(NV_PHYSICAL_GPU_HANDLE const& handle) { return loadNvidiaStruct<NVIDIA_##T_>(handle, function_); }

SIMPLE_NVIDIA_CALL(DYNAMIC_PSTATES, NVIDIA_RAW_GetDynamicPStates);
SIMPLE_NVIDIA_CALL(GPU_PSTATES20_V2, NVIDIA_RAW_GetPstates20);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_INFO, NVIDIA_RAW_GpuClientPowerPoliciesGetInfo);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_STATUS, NVIDIA_RAW_GpuClientPowerPoliciesGetStatus);
SIMPLE_NVIDIA_CALL(GPU_VOLTAGE_DOMAINS_STATUS, NVIDIA_RAW_GpuGetVoltageDomainsStatus);

bool NvidiaGPU::poll()
{
    auto frequencies = loadCLOCK_FREQUENCIES(this->handle);
    auto dynamicPstates = loadDYNAMIC_PSTATES(this->handle);
    auto pstates20 = loadGPU_PSTATES20_V2(this->handle);
    //auto perfTable = std::make_shared<NVIDIA_GPU_PERF_TABLE>();
    auto powerPoliciesInfo = loadGPU_POWER_POLICIES_INFO(this->handle);
    auto powerPoliciesStatus = loadGPU_POWER_POLICIES_STATUS(this->handle);
    auto voltageDomainsStatus = loadGPU_VOLTAGE_DOMAINS_STATUS(this->handle);
    auto thermalSettings = loadGPU_THERMAL_SETTINGS_V2(this->handle);

    auto success = false;

    if (frequencies && dynamicPstates && pstates20 && powerPoliciesInfo &&
        powerPoliciesStatus && voltageDomainsStatus && thermalSettings) {
        success = true;
        this->frequencies = std::move(frequencies);
        this->dynamicPstates = std::move(dynamicPstates);
        this->pstates20 = std::move(pstates20);
        this->powerPoliciesInfo = std::move(powerPoliciesInfo);
        this->powerPoliciesStatus = std::move(powerPoliciesStatus);
        this->voltageDomainsStatus = std::move(voltageDomainsStatus);
        this->thermalSettings = std::move(thermalSettings);
    }

    return success;
}

std::string NvidiaGPU::getName()
{
    char name_buf[64];
    
    if (NVIDIA_RAW_GetFullName(this->handle, name_buf) != NVAPI_OK) {
        name_buf[0] = '\0';
    }

    return std::string(name_buf);
}

float NvidiaGPU::getVoltage()
{
    if (this->voltageDomainsStatus && this->voltageDomainsStatus->count > 0) {
        for (int i = 0; i < this->voltageDomainsStatus->count; i++)
        {
            if (this->voltageDomainsStatus->entries[i].voltage_domain == 0) {
                return this->voltageDomainsStatus->entries[i].current_voltage / 1000.0;
            }
        }
    }
    return -1;
}

float NvidiaGPU::getTemp()
{
    if (this->thermalSettings && this->thermalSettings->count > 0) {
        for (int i = 0; i < this->thermalSettings->count; i++)
        {
            if (this->thermalSettings->sensor[i].target == NVIDIA_THERMAL_TARGET_GPU) {
                return this->thermalSettings->sensor[i].current_temp;
            }
        }
    }
    return -1;
}

std::unique_ptr<GpuClocks> NvidiaGPU::getClocks()
{
    auto fetcher = [&](NVIDIA_CLOCK_SYSTEM system) -> float {
        if (this->frequencies->entries[system].present) {
            return this->frequencies->entries[system].freq / 1000.0;
        }
        return -1;
    };

    auto clocks = new GpuClocks {
        fetcher(NVIDIA_CLOCK_SYSTEM_GPU),
        fetcher(NVIDIA_CLOCK_SYSTEM_MEMORY),
        fetcher(NVIDIA_CLOCK_SYSTEM_SHADER)
    };
    return std::unique_ptr<GpuClocks>(clocks);
}

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
    auto fetcher = [&](int i) { return GpuOverclockSetting(best_pstate.clocks[i].freq_delta, (best_pstate.flags & 1)); };

    int gpu_voltage_domain = INT_MAX;

    for (int i = 0; i < this->pstates20->clock_count; i++)
    {
        auto clock = best_pstate.clocks[i];
        switch (clock.domain) {
        case NVIDIA_CLOCK_SYSTEM_GPU:
            profile->coreOverclock = fetcher(i);
            if (clock.type == 1) {
                gpu_voltage_domain = clock.voltage_domain;
            }
            break;
        case NVIDIA_CLOCK_SYSTEM_MEMORY:
            profile->memoryOverclock = fetcher(i);
            break;
        case NVIDIA_CLOCK_SYSTEM_SHADER:
            profile->shaderOverclock = fetcher(i);
            break;
        }

    }

    if (gpu_voltage_domain < INT_MAX) {
        auto over_volt = this->pstates20->over_volt;
        for (int i = 0; i < over_volt.voltage_count; i++)
        {
            if (over_volt.voltages[i].domain == gpu_voltage_domain) {
                profile->overvolt = GpuOverclockSetting(over_volt.voltages[i].volt_delta, (over_volt.voltages[i].flags & 1));
            }
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

bool NvidiaGPU::setOverclock(const GpuOverclockDefinitionMap& overclockDefinitions)
{
    if(!this->dynamicPstates) {
        if (!this->poll()) {
            return false;
        }
    }

    auto old_profile = this->getOverclockProfile();
    auto overvolt_count = overclockDefinitions.find(GPU_OVERCLOCK_SETTING_AREA_OVERVOLT) != overclockDefinitions.end() ? 1 : 0;
    auto clock_count = overclockDefinitions.size() - overvolt_count;

    NVIDIA_GPU_PSTATES20_V2 pstates;
    REINIT_NVIDIA_STRUCT(pstates);

    int pstate_num = 0;
    pstates.clock_count = clock_count;
    pstates.state_count = 1;
    auto& state = pstates.states[0];
    state.state_num = pstate_num;

    int clock = 0;

    for each (const auto& var in overclockDefinitions)
    {
        bool is_clock = true;
        int domain = INT_MAX;
        UINT32 new_value = var.second;
        UINT32 raw_new_value = new_value * 1000;
        
        auto old_setting = old_profile->operator[](var.first);
        if (!(old_setting.editable && new_value <= old_setting.maxValue && new_value >= old_setting.minValue)) {
            return false;
        }
        
        if (var.first != GPU_OVERCLOCK_SETTING_AREA_OVERVOLT) {
            switch (var.first) {
            case GPU_OVERCLOCK_SETTING_AREA_CORE:
                domain = NVIDIA_CLOCK_SYSTEM_GPU;
                break;
            case GPU_OVERCLOCK_SETTING_AREA_MEMORY:
                domain = NVIDIA_CLOCK_SYSTEM_MEMORY;
                break;
            case GPU_OVERCLOCK_SETTING_AREA_SHADER:
                domain = NVIDIA_CLOCK_SYSTEM_SHADER;
                break;
            default:
                return false;
            }           

            is_clock = true;
        } else {
            is_clock = false;
            for (int i = 0; i < this->pstates20->state_count; i++)
            {
                if (this->pstates20->states[i].state_num != pstate_num) {
                    continue;
                }

                for (int j = 0; j < this->pstates20->clock_count; j++)
                {
                    if (this->pstates20->states[i].clocks[j].domain == NVIDIA_CLOCK_SYSTEM_GPU &&
                        this->pstates20->states[i].clocks[j].type == 1) {
                        domain = this->pstates20->states[i].clocks[j].voltage_domain;
                    }
                }
            }
        }

        if (domain == INT_MAX) {
            return false;
        }

        if (is_clock) {
            auto& clock_entry = state.clocks[clock++];
            clock_entry.domain = domain;
            clock_entry.freq_delta.value = raw_new_value;
        } else {
            pstates.over_volt.voltage_count = 1;
            auto& ov_entry = pstates.over_volt.voltages[0];
            ov_entry.domain = domain;
            ov_entry.volt_delta.value = raw_new_value;
        }
        
    }

    return NVIDIA_RAW_SetPstates20(this->handle, &pstates) == NVAPI_OK;
}

NvidiaGPU::NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle)
{
    this->handle = handle;
}