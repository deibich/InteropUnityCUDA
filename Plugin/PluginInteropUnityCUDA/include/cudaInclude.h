#pragma once
#include "cuda_runtime.h"

// use this macro if you want to check cuda function
#define CUDA_CHECK(ans) { gpuAssert((ans), __FILE__, __LINE__); }

inline void gpuAssert(cudaError_t code, const char* file, int line)
{
    if (code != cudaSuccess)
    {
        char buffer[2048];
        sprintf_s(buffer, "Cuda error: %i %s %s %d\n", code, cudaGetErrorString(code), file, line);
        std::string strError(buffer);
        Log::log().debugLogError(buffer);
    }
}
