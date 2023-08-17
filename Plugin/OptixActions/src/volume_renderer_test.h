#pragma once
#include "Texture.h"
#include "owl/common.h"
#include "owl/owl.h"

struct SourceRegion;

class VolumeRendererTest
{
public:
    explicit VolumeRendererTest(Texture* texture);

    ~VolumeRendererTest();

    void setTexture(Texture* texture);
    void render(void* data);

private:
    Texture* _texture{ nullptr };
    owl::common::vec2i _fbSize{0, 0};

    OWLContext _context{ nullptr };
    OWLModule _module{ nullptr };
    OWLGroup _world{ nullptr };
    OWLGroup _aabbWorld{ nullptr };
    OWLRayGen _rayGen{ nullptr };
    OWLParams _launchParameters{ nullptr };
    OWLMissProg _missProg{ nullptr };
    OWLBuffer _surfaceBuffer{ nullptr };
    OWLBuildSBTFlags _buildSbtFlags{ OWLBuildSBTFlags::OWL_SBT_GEOMS };
};

class SourceVolumeRendererTest
{
public:
    explicit SourceVolumeRendererTest(Texture* texture, std::vector<SourceRegion> &sourceRegions, std::vector<float> &sourceRegionDataBuffer);

    ~SourceVolumeRendererTest();

    void setTexture(Texture* texture);
    void render(void* data);

private:
    Texture* _texture{ nullptr };
    owl::common::vec2i _fbSize{0, 0};

    OWLContext _context{ nullptr };
    OWLModule _module{ nullptr };
    OWLGroup _world{ nullptr };
    OWLGroup _aabbWorld{ nullptr };
    OWLRayGen _rayGen{ nullptr };
    OWLParams _launchParameters{ nullptr };
    OWLMissProg _missProg{ nullptr };
    OWLBuffer _surfaceBuffer{ nullptr };
    OWLBuildSBTFlags _buildSbtFlags{ OWLBuildSBTFlags::OWL_SBT_GEOMS };
};