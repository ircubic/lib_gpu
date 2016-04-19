#pragma once

#include <ostream>
#include <iomanip>
#include <sstream>
#include "nvidia_interface_datatypes.h"

namespace lib_gpu {

NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_DELTA_ENTRY &d);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_PSTATES20_V2 &p);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_CLOCK_FREQUENCIES &f);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_PERF_TABLE &p);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_DYNAMIC_PSTATES &p);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_POWER_POLICIES_INFO &p);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_POWER_POLICIES_STATUS &p);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_VOLTAGE_DOMAINS_STATUS &v);
NVLIB_EXPORTED std::ostream& operator<<(std::ostream &strm, const NVIDIA_GPU_THERMAL_SETTINGS_V2 &t);

}