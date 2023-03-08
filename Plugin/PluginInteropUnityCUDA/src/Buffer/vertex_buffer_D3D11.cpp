#include "vertex_buffer_D3D11.h"
#include "d3d11_include.h"
#include <cuda_d3d11_interop.h>

#if SUPPORT_D3D11

VertexBuffer_D3D11::VertexBuffer_D3D11(void *bufferHandle, int size)
    : VertexBuffer(bufferHandle, size)
{
}

VertexBuffer_D3D11::~VertexBuffer_D3D11()
{
    // final check to be sure there was no mistake
    unRegisterBufferInCUDA();
    CUDA_CHECK(cudaGetLastError());
};

/// <summary>
/// Register the buffer from D3D11 to CUDA
/// </summary>
void VertexBuffer_D3D11::registerBufferInCUDA()
{
    // Every ID3D11Buffer is a ID3D11Resource
    const auto iD3D11Resource = static_cast<ID3D11Resource *>(_bufferHandle);

    // Valid flags for d3d11
    // cudaGraphicsRegisterFlagsNone, cudaGraphicsRegisterFlagsSurfaceLoadStore, cudaGraphicsRegisterFlagsTextureGather
    // https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__D3D11.html#group__CUDART__D3D11_1g85d07753780643584b8febab0370623b
    CUDA_CHECK(cudaGraphicsD3D11RegisterResource(&_pGraphicsResource, iD3D11Resource, cudaGraphicsRegisterFlagsNone))
}

void VertexBuffer_D3D11::unRegisterBufferInCUDA()
{
    if (_pGraphicsResource)
    {
        CUDA_CHECK(cudaGraphicsUnregisterResource(_pGraphicsResource))
        _pGraphicsResource = nullptr;
    }
    
}

#endif
