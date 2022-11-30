#pragma once
#include "ActionSampleTexture.h"
#include "action.h"
#include "unityPlugin.h"
#include <memory>


namespace SampleBasic {

	ActionSampleTexture::ActionSampleTexture(void* texturePtr, int width, int height) : Action()
	{
		_texture = CreateTextureInterop(texturePtr, width, height);
		_hasBeenRegistered = false;
	}

	ActionSampleTexture::~ActionSampleTexture()
	{
		_texture->unRegisterTextureInCUDA();
	}

	inline int ActionSampleTexture::Start()
	{
		_texture->registerTextureInCUDA();
		return 0;
	}

	int ActionSampleTexture::Update()
	{		
		cudaSurfaceObject_t surf = _texture->mapTextureToSurfaceObject();
		_texture->writeTexture(surf, GetTime());
		_texture->unMapTextureToSurfaceObject(surf);
		return 0;
	}

	inline int ActionSampleTexture::OnDestroy()
	{
		return 0;
	}

} // namespace SampleBasic


extern "C" {

	UNITY_INTERFACE_EXPORT SampleBasic::ActionSampleTexture* UNITY_INTERFACE_API createActionSampleTextureBasic(void* texturePtr, int width, int height)
	{
		return (new SampleBasic::ActionSampleTexture(texturePtr, width, height));
	}
} // extern "C"