// TestApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "lib_gpu.h"
#include "nvidia_interface.h"
#include "nvidia_simple_api.h"

using namespace lib_gpu;
using namespace lib_gpu::nvidia_simple_api;

int main()
{
    int flags = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );
    _CrtSetDbgFlag(flags | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_CHECK_ALWAYS_DF | _CRTDBG_DELAY_FREE_MEM_DF);
    /**
     * Just a debugging/data dumping application, excuse the mess :)
     */
    GpuOverclockProfile profile = get_overclock_profile(0);
    GpuClocks clocks = get_clocks(0);
    GpuClocks clocks1 = get_clocks(1);
    GpuUsage usages = get_usages(0);
    auto api = NvidiaApi();
    auto gpu = api.getGPU(0);
    gpu->poll();
    bool success = false;
    /*auto new_overclock = GpuOverclockDefinitionMap();
    new_overclock[GPU_OVERCLOCK_SETTING_AREA_CORE] = 140;
    new_overclock[GPU_OVERCLOCK_SETTING_AREA_OVERVOLT] = 87;
    bool success = gpu->setOverclock(new_overclock);
    auto profile2 = get_overclock_profile();*/
    //bool overclock_success = overclock(140, GPU_OVERCLOCK_SETTING_AREA_MEMORY);

    std::cout << "GPU Clock: " << clocks.coreClock << std::endl
        << "Mem Clock: " << clocks.memoryClock << std::endl
        << "Usage: " << usages.coreUsage << "%" << std::endl
        << "Voltage: " << gpu->getVoltage() << "mV" << std::endl
        << "Temp: " << gpu->getTemp() << "C" << std::endl;
    
    char buffer[1024];
    memset(buffer, 0, 1024);
    NV_ASSERT(NVIDIA_RAW_GetVersionString(buffer));
    std::cout << buffer << std::endl;

    NV_PHYSICAL_GPU_HANDLE handles[32];
    unsigned long count;
    NV_ASSERT(NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count));
    NV_PHYSICAL_GPU_HANDLE handle = handles[0];


    NV_ASSERT(NVIDIA_RAW_GetFullName(handle, buffer));
    std::cout << buffer << std::endl;

    success = get_name(0, buffer);
    std::cout << buffer << std::endl;

    std::cout << std::is_pod<GpuOverclockProfile>::value << std::endl;

    NVIDIA_GPU_POWER_POLICIES_INFO p;
    REINIT_NVIDIA_STRUCT(p);
    NV_ASSERT(NVIDIA_RAW_GpuClientPowerPoliciesGetInfo(handle, &p));

    NVIDIA_GPU_POWER_POLICIES_STATUS p_s;
    REINIT_NVIDIA_STRUCT(p_s);
    NV_ASSERT(NVIDIA_RAW_GpuClientPowerPoliciesGetStatus(handle, &p_s));

    NVIDIA_GPU_PSTATES20_V2 pstates;
    REINIT_NVIDIA_STRUCT(pstates);
    NV_ASSERT(NVIDIA_RAW_GetPstates20(handle, &pstates));

    NVIDIA_DYNAMIC_PSTATES dynamic_pstates;
    memset(dynamic_pstates.pstates, 0, 16 * 4);
    NV_ASSERT(NVIDIA_RAW_GetDynamicPStates(handle, &dynamic_pstates));

    std::cout << "GPU: " << dynamic_pstates.pstates[NVIDIA_DYNAMIC_PSTATES_SYSTEM_GPU].value << "%" << std::endl
        << "FB: " << dynamic_pstates.pstates[NVIDIA_DYNAMIC_PSTATES_SYSTEM_FB].value << "%" << std::endl
        << "VID: " << dynamic_pstates.pstates[NVIDIA_DYNAMIC_PSTATES_SYSTEM_VID].value << "%" << std::endl
        << "BUS: " << dynamic_pstates.pstates[NVIDIA_DYNAMIC_PSTATES_SYSTEM_BUS].value << "%" << std::endl;

    NVIDIA_CLOCK_FREQUENCIES clock_freqs, boost_clock_freqs, base_clock_freqs;
    ZeroMemory(clock_freqs.entries, 32 * 8);
    ZeroMemory(boost_clock_freqs.entries, 32 * 8);
    ZeroMemory(base_clock_freqs.entries, 32 * 8);
    clock_freqs.clock_type = 0;
    boost_clock_freqs.clock_type = 2;
    base_clock_freqs.clock_type = 1;

    NV_ASSERT(NVIDIA_RAW_GetAllClockFrequencies(handle, &clock_freqs));
    NV_ASSERT(NVIDIA_RAW_GetAllClockFrequencies(handle, &boost_clock_freqs));
    NV_ASSERT(NVIDIA_RAW_GetAllClockFrequencies(handle, &base_clock_freqs));

    std::cout << "Raw values:" << std::endl;
    std::cout << "Current clock: " << clock_freqs.entries[0].freq / 1000.0 << std::endl;
    std::cout << "Base clock: " << base_clock_freqs.entries[0].freq / 1000.0 << std::endl;
    std::cout << "Boost clock: " << boost_clock_freqs.entries[0].freq / 1000.0 << std::endl;

    std::cout <<std::endl << "API values:" << std::endl;
    std::cout << "Current clock: " << gpu->getClocks()->coreClock << std::endl;
    std::cout << "Base clock: " << gpu->getBaseClocks()->coreClock << std::endl;
    std::cout << "Boost clock: " << gpu->getBoostClocks()->coreClock << std::endl;

    NVIDIA_GPU_PERF_TABLE table;
    INIT_NVIDIA_STRUCT(table, 1);
    NV_ASSERT(NVIDIA_RAW_GetPerfClocks(handle, 1, &table));

    NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS status;
    REINIT_NVIDIA_STRUCT(status);
    NV_ASSERT(NVIDIA_RAW_GpuGetVoltageDomainsStatus(handle, &status));

    NVIDIA_GPU_THERMAL_SETTINGS_V2 settings;
    REINIT_NVIDIA_STRUCT(settings);
    NV_ASSERT(NVIDIA_RAW_GpuGetThermalSettings(handle, NVIDIA_THERMAL_TARGET_ALL, &settings));

    int check = _CrtCheckMemory();
    return 0;
}

