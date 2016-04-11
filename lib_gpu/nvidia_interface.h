#pragma once
#include <Windows.h>
#include "helpers.h"
#include "nvidia_interface_datatypes.h"


#pragma region Helpers

#define ZERO_STRUCT(s) do {memset(&s, 0, sizeof(s));} while(0)
#define INIT_NVIDIA_STRUCT(_struct, _version) do {ZERO_STRUCT(_struct); _struct.version = NVIDIA_STRUCT_VERSION(_struct, _version);} while(0)
#define REINIT_NVIDIA_STRUCT(_struct) do{UINT32 version = _struct.version; ZERO_STRUCT(_struct); _struct.version = version;} while(0)
#define NV_ASSERT(x) do {NV_STATUS ret = x; if(ret != NVAPI_OK) {return ret;}} while(0)

#pragma endregion



#pragma region Exports
#ifdef __cplusplus
namespace lib_gpu {
extern "C" {
#endif
    NVLIB_EXPORTED int init_library();
#include "nvidia_interface_gen.h"
#ifdef __cplusplus
}
}
#endif
#pragma endregion