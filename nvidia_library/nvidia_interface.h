#pragma once
#include <Windows.h>
#include "helpers.h"
#include "nvidia_interface_datatypes.h"


#pragma region Helpers

#define ZERO_STRUCT(s) do {memset(&s, 0, sizeof(s));} while(0)
#define INIT_NVIDIA_STRUCT(_struct, _version) do {ZERO_STRUCT(_struct); _struct.version = NVIDIA_STRUCT_VERSION(_struct, _version);} while(0)
#define NV_ASSERT(x) do {NV_STATUS ret = x; if(ret != NVAPI_OK) {return ret;}} while(0)

#pragma endregion



#pragma region Exports

extern "C" {
    NVLIB_EXPORTED int init_library();
#include "nvidia_interface_gen.h"
}

#pragma endregion