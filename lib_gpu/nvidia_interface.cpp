#include <windows.h>
#include "pch.h"
#include "nvidia_interface.h"
#include <memory>

namespace lib_gpu {

class NvidiaLibraryHandle
{
public:
    NvidiaLibraryHandle()
    {
        bool success = false;
        library = LoadLibrary("nvapi.dll");
        if (library != nullptr) {
            nvidia_query = (void *(*)(UINT32 ID))GetProcAddress(library, "nvapi_QueryInterface");
            if (nvidia_query != nullptr) {
                success = NVIDIA_RAW_NvidiaInit() == NVAPI_OK;
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
    void * (*nvidia_query)(UINT32 ID);
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