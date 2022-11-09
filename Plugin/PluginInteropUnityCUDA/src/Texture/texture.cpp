#pragma once
#include "texture.h"


void kernelCallerWriteTexture(const dim3 dimGrid,const dim3 dimBlock, cudaSurfaceObject_t inputSurfaceObj, const float t, const int width, const int height);


Texture::Texture(void* textureHandle, int textureWidth, int textureHeight)
{
    _textureHandle = textureHandle;
    _textureWidth = textureWidth;
    _textureHeight = textureHeight;
    _dimBlock = { 8, 8, 1 };
    _dimGrid = { (textureWidth + _dimBlock.x - 1) / _dimBlock.x,
        (textureHeight + _dimBlock.y - 1) / _dimBlock.y, 1};
    _pGraphicsResource = nullptr;
    
}


cudaSurfaceObject_t Texture::mapTextureToSurfaceObject()
{
    cudaArray* arrayPtr;
    cudaGraphicsMapResources(1, &_pGraphicsResource);
    //https://docs.nvidia.com/cuda/cuda-runtime-api/group__CUDART__INTEROP.html#group__CUDART__INTEROP_1g0dd6b5f024dfdcff5c28a08ef9958031
    CUDA_CHECK(cudaGraphicsSubResourceGetMappedArray(&arrayPtr, _pGraphicsResource, 0, 0));

    // Wrap the cudaArray in a surface object
    cudaResourceDesc resDesc;
    memset(&resDesc, 0, sizeof(resDesc));
    resDesc.resType = cudaResourceTypeArray;
    resDesc.res.array.array = arrayPtr;
    cudaSurfaceObject_t inputSurfObj = 0;
    CUDA_CHECK(cudaCreateSurfaceObject(&inputSurfObj, &resDesc));
    return inputSurfObj;
}

void Texture::writeTexture(cudaSurfaceObject_t& inputSurfObj, const float time)
{
    kernelCallerWriteTexture(_dimGrid, _dimBlock, inputSurfObj, time, _textureWidth, _textureHeight);
    CUDA_CHECK(cudaDeviceSynchronize());
}

void Texture::unMapTextureToSurfaceObject(cudaSurfaceObject_t& inputSurfObj)
{
    CUDA_CHECK(cudaGraphicsUnmapResources(1, &_pGraphicsResource));
    CUDA_CHECK(cudaDestroySurfaceObject(inputSurfObj));
    CUDA_CHECK(cudaGetLastError());
}

