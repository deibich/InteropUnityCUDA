#include "texture_D3D11.h"
#include "d3d11_include.h"
#include <cuda_d3d11_interop.h>

#if SUPPORT_D3D11

Texture_D3D11::Texture_D3D11(void *textureHandle, int textureWidth,
                             int textureHeight, int textureDepth)
    : Texture(textureHandle, textureWidth, textureHeight, textureDepth)
{
}

Texture_D3D11::~Texture_D3D11()
{

    // final check to be sure there was no mistake
    unRegisterTextureInCUDA();
    CUDA_CHECK(cudaGetLastError());
}

void Texture_D3D11::registerTextureInCUDA()
{
    // Every ID3D11Texture2D is a ID3D11Resource
    const auto iD3D11Resource = static_cast<ID3D11Resource*>(_textureHandle);

    // Valid flags for d3d11
    // cudaGraphicsRegisterFlagsNone, cudaGraphicsRegisterFlagsSurfaceLoadStore, cudaGraphicsRegisterFlagsTextureGather
    // https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__D3D11.html#group__CUDART__D3D11_1g85d07753780643584b8febab0370623b
    CUDA_CHECK(cudaGraphicsD3D11RegisterResource(&_pGraphicsResource, iD3D11Resource, cudaGraphicsRegisterFlagsNone))
}

void Texture_D3D11::unRegisterTextureInCUDA()
{
    if (_pGraphicsResource)
    {
        CUDA_CHECK(cudaGraphicsUnregisterResource(_pGraphicsResource))
        _pGraphicsResource = nullptr;
    }
}

#endif
