
#include "action_test_optix_owl.h"
#include "unity_plugin.h"
#include "volume_renderer_test.h"

#include <iostream>
#include <string>

#include "deviceCode.h"


OptixActions::ActionTestOptixOwl::ActionTestOptixOwl(void* texturePtr, int width, int height,
                                                   int depth)
{
    _texture = CreateTextureInterop(texturePtr, width, height, depth);
}

inline int OptixActions::ActionTestOptixOwl::Start()
{
    auto ret = _texture->registerTextureInCUDA();
    GRUMBLE(ret, "There has been an error during the registration of "
            "the texture in CUDA. Abort ActionTestOpixOwl !");

    ret = _texture->mapTextureToSurfaceObject(false);
    GRUMBLE(ret, "There has been an error during the mapping of "
                 "the surface in CUDA. Abort ActionTestOpixOwl !");
    if (ret == 0)
    {
        _owlRenderer = std::make_unique<VolumeRendererTest>(_texture);
    }
    else
    {
        Log::log().debugLogError("Renderer not started");
        return 1;
    }
    
    return 0; 
}
 
inline int OptixActions::ActionTestOptixOwl::Update()
{
    
    if (_owlRenderer != nullptr)
    {
        _owlRenderer->render(_renderingData);
    }
    return 0;
}

inline int OptixActions::ActionTestOptixOwl::OnDestroy() {
    if (_owlRenderer != nullptr)
        _owlRenderer.reset(nullptr);
    int ret = _texture->unmapTextureToSurfaceObject();
    GRUMBLE(ret, "There has been an error during the unmapping of "
                 "the surface in CUDA. Abort ActionTestOpixOwl !");
    ret = _texture->unregisterTextureInCUDA();
    GRUMBLE(ret, "There has been an error during the unregistration of "
            "the texture in CUDA. Abort ActionTestOpixOwl !");
    return 0;    
}

void OptixActions::ActionTestOptixOwl::setRenderingData(void* data) { _renderingData = data; }

void OptixActions::ActionTestOptixOwl::setTexture(void* texturePtr, int width, int height, int depth)
{
    if(_owlRenderer != nullptr)
    {
        const auto newTexture = CreateTextureInterop(texturePtr, width, height, depth);
        newTexture->registerTextureInCUDA();
        newTexture->mapTextureToSurfaceObject(false);

        _owlRenderer->setTexture(newTexture);

    	_texture->unmapTextureToSurfaceObject();
        _texture->unregisterTextureInCUDA();
        _texture = newTexture;
    }
}

// *****************


extern "C"
{
    UNITY_INTERFACE_EXPORT OptixActions::ActionTestOptixOwl*UNITY_INTERFACE_API createActionTestOpixOwl(void *texturePtr, int width, int height, int depth)
    {
        return (new OptixActions::ActionTestOptixOwl(texturePtr, width, height, depth));
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setTexture(OptixActions::ActionTestOptixOwl* actionObject, void* texturePtr, int width, int height, int depth)
    {
        actionObject->setTexture(texturePtr, width, height, depth);
    }

    UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setRenderingData(OptixActions::ActionTestOptixOwl*actionObject, void* data)
    {
        actionObject->setRenderingData(data);
    }
    
} // extern "C"
