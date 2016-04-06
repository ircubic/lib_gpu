// TestApplication.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include "nvidia_interface.h"
#include "nvidia_simple_api.h"

int main()
{

    struct nvidia_simple_clocks clocks = get_clocks();
    struct nvidia_simple_usages usages = get_usages();
    struct nvidia_simple_overclock_profile profile = get_overclock_profile();
    bool overclock_success = overclock(125, NVIDIA_CLOCK_SYSTEM_GPU);

    std::cout << "GPU Clock: " << clocks.coreClock << std::endl << "Mem Clock: " << clocks.memoryClock << std::endl << "Usage: " << usages.gpuUsage << "%" << std::endl;

    char buffer[1024];
    memset(buffer, 0, 1024);
    NV_ASSERT(NVIDIA_RAW_GetVersionString(buffer));
    std::cout << buffer << std::endl;

    NV_PHYSICAL_GPU_HANDLE handles[32];
    unsigned long count;
    NV_ASSERT(NVIDIA_RAW_GetPhysicalGPUHandles(handles, &count));

    NV_PHYSICAL_GPU_HANDLE handle = handles[0];
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

    std::cout << "Current clock: " << clock_freqs.entries[0].freq / 1000.0 << std::endl;
    std::cout << "Base clock: " << base_clock_freqs.entries[0].freq / 1000.0 << std::endl;
    std::cout << "Boost clock: " << boost_clock_freqs.entries[0].freq / 1000.0 << std::endl;

    NVIDIA_GPU_PERF_TABLE table;
    INIT_NVIDIA_STRUCT(table, 1);
    NV_ASSERT(NVIDIA_RAW_GetPerfClocks(handle, 1, &table));

    return 0;
}

