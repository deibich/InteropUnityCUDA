
#pragma once

#include "unity_plugin.h"
#include "texture.h"

class SourceVolumeRendererTest;
struct SourceRegion;
class VolumeRendererTest;
struct ID3D11Fence;
struct ID3D11DeviceContext4;
namespace OptixActions {

	class ActionSourceVolumeTest : public Action {
	public:
		ActionSourceVolumeTest(void* texturePtr, int width, int height, int depth, std::string fitsFilePath, std::string catalogFilePath);
		inline int Start() override;
		inline int Update() override;
		inline int OnDestroy() override;
		void setRenderingData(void* data);
		void setTexture(void* texturePtr, int width, int height, int depth);
	private:
		Texture* _texture;
		std::unique_ptr<SourceVolumeRendererTest> _owlRenderer;
		void* _renderingData;

		std::vector<SourceRegion> _sourceRegions{};
		std::vector<float> _sourceRegionDataBuffer{};

		ID3D11Fence* _fence1;
		ID3D11Fence* _fence2;
		ID3D11DeviceContext4* _context4;
	};

}

extern "C"
{
	UNITY_INTERFACE_EXPORT OptixActions::ActionSourceVolumeTest* UNITY_INTERFACE_API
		createActionSourceVolumeTest(void* texturePtr, int width, int height, int depth, const char* fitsFilePath, const char* catalogFilePath);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setTextureSourceVolumeTest(OptixActions::ActionSourceVolumeTest* actionObject, void* texturePtr, int width, int height, int depth);
	UNITY_INTERFACE_EXPORT void UNITY_INTERFACE_API setRenderingDataSourceVolumeTest(OptixActions::ActionSourceVolumeTest*, void* data);
}
