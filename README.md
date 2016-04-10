# lib_gpu v0.0.1

This is a library for getting information from and overclocking (NVIDIA) GPUs.
It's partially based on reverse engineering the private interfaces of the
NVIDIA nvapi.dll library to allow for functionality that the public
interfaces don't expose.

It has two main interfaces, one C++ based and one that is a simplified set of
C functions for linking with other languages via FFI solutions.

The library is currently in a slightly rough state due to having prioritized
functionality over readability and documentation. The readability and
documentation will be improved in later versions.

The library is released under the terms of the MIT license.

I make no guarantees for the health of your system when using this library, and
it may in fact lead to any of the following: your system catching on fire, your
GPU exploding, a portal to the unknown dimensions opening and releasing
hellspawn from your computer, or the end of humanity as we know it.

**Use with caution.**

## Usage examples

### Note about overclocking

Overclocking is done in delta values, meaning that when you see the value
`140` below, that means +140 over the default value, not that you're setting
the actual value to 140. Negative values are also allowed, and the limits are
specified by the overclock profile returned by the relevant API functions.

### Simplified interface

The simplified interface is available from `nvidia_simple_api.h` and must first be
initialized before you can use the other methods:

```C
bool success = init_simple_api();
```

Then you can query values:

```C
struct GpuClocks clocks = get_clocks();
```

Or set some overclock values:

```C
bool success = overclock(140, GPU_OVERCLOCK_SETTING_AREA_CORE) &&
 overclock(300, GPU_OVERCLOCK_SETTING_AREA_MEMORY) &&
 overclock(87.5, GPU_OVERCLOCK_SETTING_AREA_OVERVOLT);
```

### C++ interface

The C++ interface is available from `lib_gpu.h` and starts with
the `NvidiaApi` class and allows you to query values:

```C++
auto api = NvidiaApi()
auto gpu = api->getGPU(0);
auto clocks = gpu->getClocks();
std::cout << "Current core frequency: " << clocks->coreClock << "MHz" << std::endl;
auto usage = gpu->getUsage();
std::cout << "Current GPU usage: " << usage->gpuUsage << "%" << std::endl;
```

You can also overclock:

```C++
GpuOverclockDefinitionMap overclockMap;
overclockMap[GPU_OVERCLOCK_SETTING_AREA_CORE] = 140;
overclockMap[GPU_OVERCLOCK_SETTING_AREA_MEMORY] = 300;
overclockMap[GPU_OVERCLOCK_SETTING_AREA_OVERVOLT] = 87.5;
bool success = gpu->setOverclock(overclockMap);
```
