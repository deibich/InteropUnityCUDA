// ======================================================================== //
// Copyright 2019-2020 Ingo Wald                                            //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "owl/Object.h"

#include <owl/owl.h>
#include <owl/common/math/vec.h>

using namespace owl;

struct NativeCameraData
{
    vec3f cameraPositionWorld{0};
    vec3f cameraToUpperLeftScreen{ 0 };
    vec3f pxStepU{ 0 };
    vec3f pxStepV{ 0 };
};

struct Matrix4x4
{
    /*
    0  4  8 12
    1  5  9 13
    2  6 10 14
    3  7 11 15
    */
    float entries[16];

    owl4x3f getOwl4x3f()
    {
        return
        {
            {entries[0], entries[4], entries[8]
            },
            {entries[1], entries[5], entries[9]},
            {entries[2], entries[6], entries[10]},
            {entries[12], entries[13], entries[14]},
        };
    }
};

struct NativeCube
{
    vec3f position;
    Quaternion3f rotation;
    vec3f scale;
};

struct NativeRenderingData
{
    NativeCube nativeCube;
    int eyeCount{1};
    NativeCameraData nativeCameradata[2];
};

struct LaunchParams
{
    int eyeIndex{0};
    NativeCameraData nativeCameraData{};
};

struct SourceRegion
{
    owl::box3i gridSourceBox{ {0}, { 1 } };
    owl::box3f sourceBoxNormalized{{0}, { 1 }};
    uint64_t bufferOffset{ 0 };
};

struct DatacubeSources
{
    SourceRegion* sourceRegions;
};


/* variables for the triangle mesh geometry */
struct TrianglesGeomData
{
    /*! base color we use for the entire mesh */
    vec4f color;
    /*! array/buffer of vertex indices */
    vec3i* index;
    /*! array/buffer of vertex positions */
    vec3f* vertex;
};

/* variables for the ray generation program */
struct RayGenData
{
    cudaSurfaceObject_t* surf;
    vec2i  fbSize;
    OptixTraversableHandle world;
};

/* variables for the miss program */
struct MissProgData
{
    vec4f  color0;
    vec4f  color1;
};

struct VolumeGridData {
    float1* gridData;
    vec3i gridDims;
};
