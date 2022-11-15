﻿#include <device_launch_parameters.h>
#include <vector_functions.h>
#include <cmath>

#define PI 3.1415926f

__global__ void writeTex(cudaSurfaceObject_t surf, int width, int height, float time) {
    const unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    const unsigned int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {

        uchar4 c;
        c.x = 0;
        c.y = (unsigned char)(time *100 + 128) % 255;
        c.z = 0;
        c.w = 255;

        surf2Dwrite(c, surf, 4 * x, y);
    }
}

__global__ void writeVertexBuffer(float4* pos, int size, float time)
{
    unsigned int x = blockIdx.x * blockDim.x + threadIdx.x;
    
    // write output vertex
    if (x < size)
    {
        pos[x] = make_float4( cos(2*PI*time/x), sin(2*PI*time/x), 0.0f,1.0f);
    }
}


void kernelCallerWriteTexture(const dim3 dimGrid, const dim3 dimBlock, cudaSurfaceObject_t inputSurfaceObj, const float time, const int width, const int height)
{
    writeTex << <dimGrid, dimBlock >> > (inputSurfaceObj, width, height, time);

}


void kernelCallerWriteBuffer(const dim3 dimGrid, const dim3 dimBlock, float4* vertexPtr,const int size, const float time)
{
    writeVertexBuffer << <dimGrid, dimBlock >> > (vertexPtr, size, time);
}