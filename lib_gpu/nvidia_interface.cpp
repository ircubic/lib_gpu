#include <windows.h>
#include "pch.h"
#include "nvidia_interface.h"
#include <memory>

namespace lib_gpu {

class NvidiaLibraryHandle
{
    typedef void *(*QueryPtr)(UINT32);
    const UINT32 NVIDIA_INIT_ID = 0x0150E828;
public:
    NvidiaLibraryHandle()
    {
        bool success = false;
        library = LoadLibrary("nvapi.dll");
        if (library != nullptr) {
            nvidia_query = reinterpret_cast<QueryPtr>(GetProcAddress(library, "nvapi_QueryInterface"));
            if (nvidia_query != nullptr) {
                auto init = static_cast<NV_STATUS(*)()>(nvidia_query(NVIDIA_INIT_ID));
                success = init() == NVAPI_OK;
            }        
        }

        if (!success) {
            throw std::runtime_error("Unable to locate NVIDIA library!");
        }
    }

    ~NvidiaLibraryHandle()
    {
        NVIDIA_RAW_NvidiaUnload();
        FreeLibrary(library);
    }

    void *query(UINT32 ID)
    {
        return nvidia_query(ID);
    }

private:
    QueryPtr nvidia_query;
    HMODULE library;
};

static std::unique_ptr<NvidiaLibraryHandle> nvidia_handle;

int init_library()
{
    try {
        if (!nvidia_handle) {
            nvidia_handle = std::make_unique<NvidiaLibraryHandle>();
        }
        return true;
    }
    catch (std::runtime_error) {
        return false;
    }
}

#include "nvidia_interface_gen.cpp"
}