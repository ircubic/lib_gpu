#pragma once

#ifdef DLL_BUILDING
#define NVLIB_EXPORTED __declspec(dllexport) 
#else
#define NVLIB_EXPORTED __declspec(dllimport) 
#endif
