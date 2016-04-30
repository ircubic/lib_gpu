#include "pch.h"

#include <array>
#include <sstream>
#include <iomanip>
#include "NvidiaGPU.h"
#include "GpuDatatypes.h"
#include "nvidia_interface.h"
#include "nvidia_interface_datatypes.h"

namespace lib_gpu {

struct NvidiaGPUDataset
{
    std::array<NVIDIA_CLOCK_FREQUENCIES, NVIDIA_CLOCK_FREQUENCY_TYPE_LAST> frequencies;
    NVIDIA_DYNAMIC_PSTATES dynamicPstates;
    NVIDIA_GPU_PSTATES20_V2 pstates20;
    NVIDIA_GPU_POWER_POLICIES_INFO powerPoliciesInfo;
    NVIDIA_GPU_POWER_POLICIES_STATUS powerPoliciesStatus;
    NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS voltageDomainsStatus;
    NVIDIA_GPU_THERMAL_SETTINGS_V2 thermalSettings;
    NVIDIA_GPU_THERMAL_POLICIES_INFO_V2 thermalPoliciesInfo;
    NVIDIA_GPU_THERMAL_POLICIES_STATUS_V2 thermalPoliciesStatus;
};

#pragma region Data loading helpers
template<typename T, typename F>
bool loadNvidiaStruct(NV_PHYSICAL_GPU_HANDLE const& handle, T* structPtr, NV_STATUS(*loader)(NV_PHYSICAL_GPU_HANDLE, T*), F preparer)
{
    UINT32 version = structPtr->version;
    memset(structPtr, 0, sizeof(T));
    structPtr->version = version;
    preparer(structPtr);
    return (*loader)(handle, structPtr) == NVAPI_OK;
}

template<typename T>
bool loadNvidiaStruct(NV_PHYSICAL_GPU_HANDLE const& handle, T* structPtr, NV_STATUS(*loader)(NV_PHYSICAL_GPU_HANDLE, T*))
{
    return loadNvidiaStruct<T>(handle, structPtr, loader, [](void*) {});
}

bool loadCLOCK_FREQUENCIES(NV_PHYSICAL_GPU_HANDLE const& handle, NVIDIA_CLOCK_FREQUENCIES* structPtr, NVIDIA_CLOCK_FREQUENCY_TYPE type)
{
    return loadNvidiaStruct<NVIDIA_CLOCK_FREQUENCIES>(handle, structPtr, NVIDIA_RAW_GetAllClockFrequencies, [&](auto f) {f->clock_type = type; });
}

bool loadGPU_THERMAL_SETTINGS_V2(NV_PHYSICAL_GPU_HANDLE const& handle, NVIDIA_GPU_THERMAL_SETTINGS_V2* structPtr)
{
    auto loader_wrapper = [](auto handle, auto settings) {
        return NVIDIA_RAW_GpuGetThermalSettings(handle, NVIDIA_THERMAL_TARGET_ALL, settings);
    };

    return loadNvidiaStruct<NVIDIA_GPU_THERMAL_SETTINGS_V2>(handle, structPtr, loader_wrapper);
}

#define SIMPLE_NVIDIA_CALL(T_, function_) bool load ## T_(NV_PHYSICAL_GPU_HANDLE const& handle, NVIDIA_##T_* structPtr) { return loadNvidiaStruct<NVIDIA_##T_>(handle, structPtr, function_); }

SIMPLE_NVIDIA_CALL(DYNAMIC_PSTATES, NVIDIA_RAW_GetDynamicPStates);
SIMPLE_NVIDIA_CALL(GPU_PSTATES20_V2, NVIDIA_RAW_GetPstates20);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_INFO, NVIDIA_RAW_GpuClientPowerPoliciesGetInfo);
SIMPLE_NVIDIA_CALL(GPU_POWER_POLICIES_STATUS, NVIDIA_RAW_GpuClientPowerPoliciesGetStatus);
SIMPLE_NVIDIA_CALL(GPU_VOLTAGE_DOMAINS_STATUS, NVIDIA_RAW_GpuGetVoltageDomainsStatus);
SIMPLE_NVIDIA_CALL(GPU_THERMAL_POLICIES_INFO_V2, NVIDIA_RAW_GpuClientThermalPoliciesGetInfo);
SIMPLE_NVIDIA_CALL(GPU_THERMAL_POLICIES_STATUS_V2, NVIDIA_RAW_GpuClientThermalPoliciesGetStatus);

#pragma endregion

#pragma region Other helpers

unsigned long getGPUIDFromHandle(const NV_PHYSICAL_GPU_HANDLE handle)
{
    unsigned long value = 0;
    if (NVIDIA_RAW_GetGPUIDFromPhysicalGPU(handle, &value) != NVAPI_OK) {
        throw std::runtime_error("Unable to get GPUID for GPU");
    }
    return value;
}

template <typename F>
std::string getNvidiaString(NV_PHYSICAL_GPU_HANDLE handle, F function)
{
    char name_buf[NVIDIA_SHORT_STRING_SIZE];

    if (function(handle, name_buf) != NVAPI_OK) {
        name_buf[0] = '\0';
    }

    return std::string(name_buf);
}

GpuOverclockSetting getPowerLimit(const NVIDIA_GPU_POWER_POLICIES_INFO& info, const NVIDIA_GPU_POWER_POLICIES_STATUS& status, unsigned pstate = 0)
{
    // We make a slightly bold assumption that info and status have the same entries
    for (auto i = 0u; i < status.count; i++) {
        const auto& statusEntry = status.entries[i];
        const auto& infoEntry = info.entries[i];
        if (statusEntry.pstate == pstate) {
            return GpuOverclockSetting{
                static_cast<float>(infoEntry.min_power) / 1000.0f,
                static_cast<float>(statusEntry.power) / 1000.0f,
                static_cast<float>(infoEntry.max_power) / 1000.0f,
                infoEntry.max_power > 0
            };
        }
    }

    return {};
}

GpuOverclockSetting getThermalLimit(const NVIDIA_GPU_THERMAL_POLICIES_INFO_V2& info, const NVIDIA_GPU_THERMAL_POLICIES_STATUS_V2& status, NVIDIA_THERMAL_CONTROLLER controller = NVIDIA_THERMAL_CONTROLLER_GPU_INTERNAL)
{
    // Again assuming that info and status wouldn't report on different values
    for (auto i = 0u; i < status.count; i++) {
        const auto& statusEntry = status.entries[i];
        const auto& infoEntry = info.entries[i];

        if (static_cast<NVIDIA_THERMAL_CONTROLLER>(statusEntry.controller) == controller) {
            // Thermal policy values are multiples of 256
            return GpuOverclockSetting{
                infoEntry.min / 256.0f,
                statusEntry.value / 256.0f,
                infoEntry.max / 256.0f,
                infoEntry.max > 0
            };
        }
    }

    return{};
}

auto get_best_pstate_index(NVIDIA_GPU_PSTATES20_V2 const& pstates)
{
    auto best_pstate_index = 0u;
    auto best_pstate_state = UINT_MAX;
    for (auto i = 0u; i < pstates.state_count; i++) {
        if (pstates.states[i].state_num < best_pstate_state) {
            best_pstate_index = i;
            best_pstate_state = pstates.states[i].state_num;
        }
    }
    return best_pstate_index;
}

auto getUsageForSystem(const NVIDIA_DYNAMIC_PSTATES_SYSTEM system, const NVIDIA_DYNAMIC_PSTATES& pstates)
{
    const auto state = pstates.pstates[system];
    return state.present ? state.value : -1.0f;
}

bool makeNewPstates20(const GpuOverclockDefinitionMap& overclockDefinitions, const GpuOverclockProfile& old_profile, const NvidiaGPUDataset& dataset, NVIDIA_GPU_PSTATES20_V2& pstates)
{
    const auto overvolt_count = overclockDefinitions.find(GPU_OVERCLOCK_SETTING_AREA_OVERVOLT) != overclockDefinitions.end() ? 1u : 0u;
    auto clock_count = 0u;

    const UINT32 pstate_num = 0;
    auto& state = pstates.states[0];
    state.state_num = pstate_num;

    int clock = 0;

    for (const auto& var : overclockDefinitions) {
        bool is_clock = true;
        auto domain = UINT_MAX;
        const auto new_value = var.second;
        const auto raw_new_value = static_cast<UINT32>(new_value * 1000);
        const auto area = var.first;

        const auto old_setting = old_profile[area];
        if (!(old_setting.editable && new_value <= old_setting.maxValue && new_value >= old_setting.minValue)) {
            return false;
        }

        if (area >= GPU_OVERCLOCK_SETTING_AREA_CORE && area <= GPU_OVERCLOCK_SETTING_AREA_SHADER) {
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

            clock_count++;
            is_clock = true;
        } else if (area == GPU_OVERCLOCK_SETTING_AREA_OVERVOLT) {
            is_clock = false;
            for (auto i = 0u; i < dataset.pstates20.state_count; i++) {
                if (dataset.pstates20.states[i].state_num != pstate_num) {
                    continue;
                }

                for (auto j = 0u; j < dataset.pstates20.clock_count; j++) {
                    if (dataset.pstates20.states[i].clocks[j].domain == NVIDIA_CLOCK_SYSTEM_GPU &&
                        dataset.pstates20.states[i].clocks[j].type == 1) {
                        domain = dataset.pstates20.states[i].clocks[j].voltage_domain;
                    }
                }
            }
        } else {
            continue;
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

    pstates.clock_count = clock_count;
    pstates.state_count = 1;

    return true;
}

bool makeNewPowerStatus(const GpuOverclockDefinitionMap& overclockDefinitions, const GpuOverclockProfile& old_profile, const NvidiaGPUDataset& dataset, NVIDIA_GPU_POWER_POLICIES_STATUS& powerStatus)
{
    const auto index = overclockDefinitions.find(GPU_OVERCLOCK_SETTING_AREA_POWER_LIMIT);
    if (index != overclockDefinitions.end()) {
        const auto new_value = index->second;
        const auto old_setting = old_profile[GPU_OVERCLOCK_SETTING_AREA_POWER_LIMIT];
        if (old_setting.editable && new_value <= old_setting.maxValue && new_value >= old_setting.minValue) {
            powerStatus.count = 1;
            powerStatus.entries[0].power = static_cast<UINT32>(new_value * 1000);
            powerStatus.entries[0].pstate = 0;
            return true;
        }
    }

    return false;
}

#pragma endregion


NvidiaGPU::NvidiaGPU(const NV_PHYSICAL_GPU_HANDLE handle) : handle(handle), GPUID(getGPUIDFromHandle(handle))
{
}

NvidiaGPU::~NvidiaGPU()
{
}

bool NvidiaGPU::poll()
{
    auto newDataset = std::make_unique<NvidiaGPUDataset>();
    bool frequencySuccess = true;

    unsigned int frequencyType = 0;
    for (auto& frequencyStruct : newDataset->frequencies) {
        if (!loadCLOCK_FREQUENCIES(this->handle, &frequencyStruct, static_cast<NVIDIA_CLOCK_FREQUENCY_TYPE>(frequencyType++))) {
            frequencySuccess = false;
        }
    }

    if (frequencySuccess &&
        loadDYNAMIC_PSTATES(this->handle, &newDataset->dynamicPstates) &&
        loadGPU_PSTATES20_V2(this->handle, &newDataset->pstates20) &&
        loadGPU_POWER_POLICIES_INFO(this->handle, &newDataset->powerPoliciesInfo) &&
        loadGPU_POWER_POLICIES_STATUS(this->handle, &newDataset->powerPoliciesStatus) &&
        loadGPU_VOLTAGE_DOMAINS_STATUS(this->handle, &newDataset->voltageDomainsStatus) &&
        loadGPU_THERMAL_SETTINGS_V2(this->handle, &newDataset->thermalSettings) &&
        loadGPU_THERMAL_POLICIES_INFO_V2(this->handle, &newDataset->thermalPoliciesInfo) &&
        loadGPU_THERMAL_POLICIES_STATUS_V2(this->handle, &newDataset->thermalPoliciesStatus)
        ) {
        this->dataset = std::move(newDataset);
        return true;
    }

    return false;
}

std::string NvidiaGPU::getName() const
{
    return getNvidiaString(handle, NVIDIA_RAW_GetFullName);
}

std::string NvidiaGPU::getSerialNumber() const
{
    auto str = getNvidiaString(handle, NVIDIA_RAW_GpuGetSerialNumber);

    auto buf = std::stringstream{};
    buf << std::hex << std::setfill('0') << std::uppercase;
    for (auto chr : str) {
        auto byte = static_cast<uint8_t>(chr);
        // have to recast to at least 16 bits, otherwise it'll print as letters
        buf << std::setw(2) << static_cast<uint16_t>(byte);
    }
    return buf.str();
}

float NvidiaGPU::getVoltage() const
{
    if (this->dataset) {
        for (unsigned int i = 0; i < this->dataset->voltageDomainsStatus.count; i++) {
            if (this->dataset->voltageDomainsStatus.entries[i].voltage_domain == 0) {
                return this->dataset->voltageDomainsStatus.entries[i].current_voltage / 1'000'000.0f;
            }
        }
    }
    return -1;
}

float NvidiaGPU::getTemperature() const
{
    if (this->dataset) {
        for (unsigned int i = 0; i < this->dataset->thermalSettings.count; i++) {
            if (this->dataset->thermalSettings.sensor[i].target == NVIDIA_THERMAL_TARGET_GPU) {
                return static_cast<float>(this->dataset->thermalSettings.sensor[i].current_temp);
            }
        }
    }
    return -1;
}

bool NvidiaGPU::isTemperatureLimitPrioritized() const
{
    if (this->dataset) {
        for (auto i = 0u; i < this->dataset->thermalPoliciesStatus.count; i++) {
            const auto& status = this->dataset->thermalPoliciesStatus.entries[i];
            if (status.controller == NVIDIA_THERMAL_CONTROLLER_GPU_INTERNAL) {
                return static_cast<bool>(status.flags & 1);
            }
        }
    }

    return true;
}

unsigned long NvidiaGPU::getGPUID() const
{
    return this->GPUID;
}

std::unique_ptr<GpuClocks> NvidiaGPU::getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE type, bool compensateForOverclock) const
{
    const auto& dataSource = this->dataset->frequencies[type];
    auto overclockProfile = this->getOverclockProfile();

    auto fetcher = [&](NVIDIA_CLOCK_SYSTEM system) -> float {
        if (dataSource.entries[system].present) {
            float compensation = 0;
            if (compensateForOverclock) {
                switch (system) {
                case NVIDIA_CLOCK_SYSTEM_GPU:
                    compensation = overclockProfile->coreOverclock.currentValue;
                    break;
                case NVIDIA_CLOCK_SYSTEM_MEMORY:
                    compensation = overclockProfile->memoryOverclock.currentValue;
                    break;
                case NVIDIA_CLOCK_SYSTEM_SHADER:
                    compensation = overclockProfile->shaderOverclock.currentValue;
                    break;
                }
            }

            return dataSource.entries[system].freq / 1000.0f + compensation;
        }
        return -1;
    };
    
    return std::unique_ptr<GpuClocks>{new GpuClocks{
        fetcher(NVIDIA_CLOCK_SYSTEM_GPU),
        fetcher(NVIDIA_CLOCK_SYSTEM_MEMORY),
        fetcher(NVIDIA_CLOCK_SYSTEM_SHADER)
    }};
}

std::unique_ptr<GpuClocks> NvidiaGPU::getClocks() const
{
    return std::move(this->getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE_CURRENT));
}

std::unique_ptr<GpuClocks> NvidiaGPU::getDefaultClocks() const
{
    return this->getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE_BASE);
}

std::unique_ptr<GpuClocks> NvidiaGPU::getBaseClocks() const
{
    return this->getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE_BASE, true);
}

std::unique_ptr<GpuClocks> NvidiaGPU::getBoostClocks() const
{
    return this->getClocks(NVIDIA_CLOCK_FREQUENCY_TYPE_BOOST, true);
}

std::unique_ptr<GpuOverclockProfile> NvidiaGPU::getOverclockProfile() const
{
    auto profile = std::make_unique<GpuOverclockProfile>();
    const auto best_pstate_index = get_best_pstate_index(this->dataset->pstates20);
    const auto& best_pstate = this->dataset->pstates20.states[best_pstate_index];

    const auto fetcher = [&](auto i) {
        return GpuOverclockSetting(best_pstate.clocks[i].freq_delta, static_cast<bool>(best_pstate.flags & 1));
    };

    auto gpu_voltage_domain = UINT_MAX;

    for (auto i = 0u; i < this->dataset->pstates20.clock_count; i++) {
        const auto& clock = best_pstate.clocks[i];
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

    if (gpu_voltage_domain < UINT_MAX) {
        const auto& over_volt = this->dataset->pstates20.over_volt;
        for (auto i = 0u; i < over_volt.voltage_count; i++) {
            if (over_volt.voltages[i].domain == gpu_voltage_domain) {
                profile->overvolt = GpuOverclockSetting(over_volt.voltages[i].volt_delta, static_cast<bool>(over_volt.voltages[i].flags & 1));
            }
        }
    }

    profile->powerLimit = getPowerLimit(this->dataset->powerPoliciesInfo, this->dataset->powerPoliciesStatus);
    profile->thermalLimit = getThermalLimit(this->dataset->thermalPoliciesInfo, this->dataset->thermalPoliciesStatus);

    return profile;
}

std::unique_ptr<GpuUsage> NvidiaGPU::getUsage() const
{
    if (this->dataset) {
        return std::unique_ptr<GpuUsage>{new GpuUsage{
            getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_GPU, this->dataset->dynamicPstates),
            getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_FB, this->dataset->dynamicPstates),
            getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_VID, this->dataset->dynamicPstates),
            getUsageForSystem(NVIDIA_DYNAMIC_PSTATES_SYSTEM_BUS, this->dataset->dynamicPstates)
        }};
    }
    return nullptr;
}

bool NvidiaGPU::setOverclock(const GpuOverclockDefinitionMap& overclockDefinitions) 
{
    if (!this->dataset) {
        if (!this->poll()) {
            return false;
        }
    }

    const auto old_profile = this->getOverclockProfile();

    NVIDIA_GPU_PSTATES20_V2 pstates;
    REINIT_NVIDIA_STRUCT(pstates);
    NVIDIA_GPU_POWER_POLICIES_STATUS powerStatus;
    REINIT_NVIDIA_STRUCT(powerStatus);
    
    auto overclockAttempted = false;
    auto overclockSuccess = true;

    auto loadWithMethod = [&](auto& dataStruct, auto method) {
        return method(overclockDefinitions, *old_profile, *this->dataset, dataStruct);
    };

    auto overclockIfValid = [&](auto& dataStruct, bool valid, auto rawMethod) {
        if (valid) {
            overclockAttempted = true;
            overclockSuccess &= (rawMethod(this->handle, &dataStruct) == NVAPI_OK);
        }
    };

    if (loadWithMethod(pstates, makeNewPstates20) && loadWithMethod(powerStatus, makeNewPowerStatus)) {
        overclockIfValid(pstates, (pstates.clock_count > 0 || pstates.over_volt.voltage_count > 0), NVIDIA_RAW_SetPstates20);
        overclockIfValid(powerStatus, powerStatus.count > 0, NVIDIA_RAW_GpuClientPowerPoliciesSetStatus);

        if (overclockAttempted) {
            this->poll();
        }

        return overclockAttempted && overclockSuccess;
    }

    return false;
}

}