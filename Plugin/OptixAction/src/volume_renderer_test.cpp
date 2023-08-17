#include "volume_renderer_test.h"
#include "deviceCode.h"
#include "owl/common.h"
#include "owl/Object.h"
#include "owl/owl.h"
#include "owl/common/math/AffineSpace.h"

#include <iostream>

#include <map>

extern "C" char deviceCode_ptx[];

VolumeRendererTest::VolumeRendererTest(Texture* texture) : _texture(texture)
{
    _context = owlContextCreate(nullptr, 1);
    _module = owlModuleCreate(_context, deviceCode_ptx);

    // Raygen
    {
        OWLVarDecl rayGenVars[] = {
            {"surf", OWL_BUFPTR, OWL_OFFSETOF(RayGenData, surf)},
            {"fbSize", OWL_INT2, OWL_OFFSETOF(RayGenData, fbSize)},
            {"world", OWL_GROUP, OWL_OFFSETOF(RayGenData, world)},
            {}
        };

        _rayGen =
            owlRayGenCreate(_context, _module, "simpleRayGen", sizeof(RayGenData), rayGenVars, -1);
    }

    setTexture(texture);

    // Create AABB 
    {
        OWLVarDecl aabbGeomsVar[] = {
		    {"sourceRegions", OWL_BUFPTR, OWL_OFFSETOF(DatacubeSources, sourceRegions)},
		    { /* sentinel to mark end of list */ }
        };

        OWLGeomType aabbGeomType = owlGeomTypeCreate(_context, OWL_GEOM_USER, sizeof(DatacubeSources), aabbGeomsVar, -1);

        // Programs for user geom must be set explicit
        owlGeomTypeSetBoundsProg(aabbGeomType, _module, "AABBGeom");
        owlGeomTypeSetIntersectProg(aabbGeomType, 0, _module, "AABBGeom");
        owlGeomTypeSetClosestHit(aabbGeomType, 0, _module, "AABBGeom");

        owlBuildPrograms(_context);

        std::vector<SourceRegion> sourceRegions;
        
        SourceRegion sr{};
        sr.sourceBoxNormalized = { {-.5f, -.5f, -.5f}, {.5f, .5f, .5f} };
        sourceRegions.push_back(sr);

        OWLBuffer owlSourcesDataBuffer = owlDeviceBufferCreate(_context, OWL_USER_TYPE(SourceRegion), 1, &sr);

        OWLGeom aabbGeom = owlGeomCreate(_context, aabbGeomType);

        owlGeomSetBuffer(aabbGeom, "sourceRegions", owlSourcesDataBuffer);
        owlGeomSetPrimCount(aabbGeom, sourceRegions.size());
        
        OWLGroup aabbGroup = owlUserGeomGroupCreate(_context, 1, &aabbGeom);
        owlGroupBuildAccel(aabbGroup);


        _aabbWorld = owlInstanceGroupCreate(_context, 1, &aabbGroup, nullptr, nullptr,
            OWL_MATRIX_FORMAT_OWL, OPTIX_BUILD_FLAG_ALLOW_UPDATE);

    	owlGroupBuildAccel(_aabbWorld);
    }

    // Launch parameters
    {
        OWLVarDecl launchParamsVarsWithStruct[] = {
            {"eyeIndex", OWL_INT, OWL_OFFSETOF(LaunchParams, eyeIndex)},
            {"nativeCameraData", OWL_USER_TYPE(NativeCameraData), OWL_OFFSETOF(LaunchParams, nativeCameraData)},
            {}
        };
        
        _launchParameters =
            owlParamsCreate(_context, sizeof(LaunchParams), launchParamsVarsWithStruct, -1);
    }
    
    // Miss Program
    {
        OWLVarDecl missProgVars[] = {
            {"color0", OWL_FLOAT3, OWL_OFFSETOF(MissProgData, color0)},
            {"color1", OWL_FLOAT3, OWL_OFFSETOF(MissProgData, color1)},
            { }
        };

        _missProg =
            owlMissProgCreate(_context, _module, "miss", sizeof(MissProgData), missProgVars, -1);

        // ----------- set variables  ----------------------------
        owlMissProgSet3f(_missProg, "color0", owl3f{.8f, 0.f, 0.f});
        owlMissProgSet3f(_missProg, "color1", owl3f{.8f, .8f, .8f});
    }

    owlBuildPrograms(_context);
    owlBuildPipeline(_context);

    owlRayGenSetBuffer(_rayGen, "surf", _surfaceBuffer);
    owlRayGenSet2i(_rayGen, "fbSize", _fbSize.x, _fbSize.y);

    _world = _aabbWorld;
    owlRayGenSetGroup(_rayGen, "world", _world);

    owlBuildSBT(_context);
    _buildSbtFlags = OWLBuildSBTFlags::OWL_SBT_GEOMS;
}

VolumeRendererTest::~VolumeRendererTest()
{
    if (_context != nullptr)
    {
        owlContextDestroy(_context);
    }
    _texture = nullptr;
}

void VolumeRendererTest::setTexture(Texture* texture)
{
    // cudaDeviceSynchronize(); // TODO: Required?
    _texture = texture;
    _fbSize = { texture->getWidth(), texture->getHeight() };

    if (_context != nullptr)
    {
        if (_surfaceBuffer == nullptr)
        {
            _surfaceBuffer = owlDeviceBufferCreate(
                _context,
                OWL_USER_TYPE(cudaSurfaceObject_t),
                _texture->getDepth(),
                _texture->getSurfaceObjectVectorDataPointer()
            );
        }
        else
        {
            owlBufferUpload(
                _surfaceBuffer,
                _texture->getSurfaceObjectVectorDataPointer(),
                0,
                _texture->getDepth()
            );
        }

        if (_rayGen != nullptr)
        {
            owlRayGenSetBuffer(_rayGen, "surf", _surfaceBuffer);
            owlRayGenSet2i(_rayGen, "fbSize", _fbSize.x, _fbSize.y);
            _buildSbtFlags = OWLBuildSBTFlags::OWL_SBT_ALL;
        }
    }
}

void VolumeRendererTest::render(void* data)
{    
    NativeRenderingData *nrd;  
    auto eyeCount{1};

    if (data != nullptr)
    {
        nrd = static_cast<NativeRenderingData*>(data);
        eyeCount = nrd->eyeCount;
        auto translate = affine3f::translate(nrd->nativeCube.position);
        auto scale = affine3f::scale(nrd->nativeCube.scale);

        AffineSpace3f rotate{ Quaternion3f{nrd->nativeCube.rotation.k, vec3f{nrd->nativeCube.rotation.r, nrd->nativeCube.rotation.i, nrd->nativeCube.rotation.j}} };
        auto trs = translate * rotate * scale;

        owlInstanceGroupSetTransform(_world, 0, (const float*)&trs);
        auto l = trs.l;
        auto r = trs.p;
        owlGroupRefitAccel(_world);
        
        owlBuildSBT(_context, _buildSbtFlags);
        for (auto i = 0; i < eyeCount; i++)
        {
            
            owlParamsSet1i(_launchParameters, "eyeIndex", i);
            owlParamsSetRaw(_launchParameters, "nativeCameraData", &nrd->nativeCameradata[i]);
            owlAsyncLaunch2D(_rayGen, _fbSize.x, _fbSize.y, _launchParameters);
        }
        owlLaunchSync(_launchParameters);
    }
    else
    {
    	owlLaunch2D(_rayGen, _fbSize.x, _fbSize.y, _launchParameters);
    }
    _buildSbtFlags = OWLBuildSBTFlags::OWL_SBT_GEOMS;

}



