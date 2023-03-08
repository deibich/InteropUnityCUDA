#pragma once

#include "texture.h"

#if SUPPORT_D3D11

class Texture_D3D11 : public Texture
{
public:
    Texture_D3D11(void *textureHandle, int textureWidth,
                         int textureHeight, int textureDepth);
    virtual ~Texture_D3D11();
    void registerTextureInCUDA() override;
    void unRegisterTextureInCUDA() override;

};

#endif
