#include <windows.h>
#include "pch.h"
#include "nvidia_interface.h"

namespace lib_gpu {
static void * (*nvidia_query)(UINT32 ID) = nullptr;
static HMODULE library = nullptr;

int init_library() {
	int success = 0;
	library = LoadLibrary("nvapi.dll");
	if (library != nullptr) {
		nvidia_query = (void *(*)(UINT32 ID))GetProcAddress(library, "nvapi_QueryInterface");
		success = (nvidia_query != nullptr);
	}
	return success;
}

#include "nvidia_interface_gen.cpp"
}