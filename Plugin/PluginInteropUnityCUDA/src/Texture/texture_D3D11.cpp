#pragma once
#include "texture_D3D11.h"

#if SUPPORT_D3D11

Texture_D3D11::Texture_D3D11(void *textureHandle, int textureWidth,
                             int textureHeight, int textureDepth,
                             RenderAPI *renderAPI)
    : Texture(textureHandle, textureWidth, textureHeight, textureDepth)
{

    // we need the render api associated to dx11, because we need device to
    // initialize texture
    _renderAPI = (RenderAPI_D3D11 *)renderAPI;
}

Texture_D3D11::~Texture_D3D11()
{
    // _texBufferInterop->Release();
    CUDA_CHECK(cudaGetLastError());
};

/// <summary>
/// Has to be call after the first issue plugin event
/// see. https://docs.unity3d.com/ScriptReference/GL.IssuePluginEvent.html
/// register a graphics resources defined from the texture dx11
/// </summary>
void Texture_D3D11::registerTextureInCUDA()
{

    // we cast it here, to make it only once.
    if(_textureDepth < 1)
    {
        auto texUnityDX11 = (ID3D11Texture2D *)_textureHandle;
        CUDA_CHECK(cudaGraphicsD3D11RegisterResource(
                &_pGraphicsResource, texUnityDX11, cudaGraphicsRegisterFlagsNone));
        CUDA_CHECK(cudaGetLastError());
    }
    else
    {
        auto texUnityDX11 = (ID3D11Texture3D *)_textureHandle;
        CUDA_CHECK(cudaGraphicsD3D11RegisterResource(
                &_pGraphicsResource, texUnityDX11, cudaGraphicsRegisterFlagsNone));
        CUDA_CHECK(cudaGetLastError());
    }
    // assert(texUnityDX11);
    // D3D11_TEXTURE2D_DESC texDesc;
    // texUnityDX11->GetDesc(&texDesc);

    // DXGI_FORMAT format = texDesc.Format;
    // Log::log().debugLog(std::to_string(format));

    // CUDA_CHECK(cudaGetLastError());
    // register the texture to cuda : it initialize the _pGraphicsResource
    
}

void Texture_D3D11::unRegisterTextureInCUDA()
{

    CUDA_CHECK(cudaGraphicsUnregisterResource(_pGraphicsResource));
}

void Texture_D3D11::copyUnityTextureToAPITexture()
{
    _renderAPI->copyTextures2D(_texBufferInterop, _texUnityDX11);
}

void Texture_D3D11::copyAPITextureToUnityTexture()
{
    _renderAPI->copyTextures2D(_texUnityDX11, _texBufferInterop);
}

#endif // #if SUPPORT_D3D11
