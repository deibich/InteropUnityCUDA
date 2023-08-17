#pragma once
#include "action.h"
#include "texture.h"

class SourceVolumeRendererTest;
struct SourceRegion;
class VolumeRendererTest;

namespace OptixActions {
	class ActionTestOptixOwl : public Action {
	public:
		ActionTestOptixOwl(void* texturePtr, int width, int height, int depth);
		inline int Start() override;
		inline int Update() override;
		inline int OnDestroy() override;
		void setRenderingData(void* data);
		void setTexture(void* texturePtr, int width, int height, int depth);
	private:
		Texture* _texture;
		int _frameIndex;
		std::unique_ptr<VolumeRendererTest> _owlRenderer;
		void* _renderingData;
	};
	

} // namespace SampleBasic

extern "C" {
	UNITY_INTERFACE_EXPORT OptixActions::ActionTestOptixOwl* UNITY_INTERFACE_API
		createActionTestOptixOwl(void* texturePtr, int width, int height, int depth);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setTexture(OptixActions::ActionTestOptixOwl* actionObject, void* texturePtr, int width, int height, int depth);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setRenderingData(OptixActions::ActionTestOptixOwl*, void* data);

}