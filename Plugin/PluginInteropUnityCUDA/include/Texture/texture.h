#pragma once
#include "IUnityGraphics.h"
#include "cuda_include.h"
#include "log.h"

class Texture
{
    public:
    /**
     * Constructor of texture
     * @param  textureHandle A pointer of texture with float4 that has
     * been generated with Unity (see function GetNativeTexturePtr
     * https://docs.unity3d.com/ScriptReference/Texture.GetNativeTexturePtr.html)
     * @param  textureWidth  the width of the texture
     * @param  textureHeight the height of the texture
     * @param  textureDepth  the depth of the texture (should be greater than 0)
     */
    UNITY_INTERFACE_EXPORT Texture(void *textureHandle, int textureWidth,
                                   int textureHeight, int textureDepth);

    /**
     * Destructor of texture, will free _surfObjArray
     */
    UNITY_INTERFACE_EXPORT ~Texture();

    /**
     * Register the texture in CUDA, this has to be override because it depends
     *  on the graphics api
     */
    UNITY_INTERFACE_EXPORT virtual int registerTextureInCUDA() = 0;

    /**
     * Unregistered the texture in CUDA, this has to be override because it
     *  depends on the graphics api
     */
    UNITY_INTERFACE_EXPORT virtual int unregisterTextureInCUDA() = 0;

    /**
     * For some API (DX11) CUDA cannot edit the texture created by Unity
     * therefore, we have to create a new texture that will used as a buffer
     * between the unity texture and the surface object that is modify by CUDA
     * For these API, this function will copy the content of the unity texture
     * to this buffer, for the other API. It'll do nothing. If Unity texture has
     * been modify in Unity, you have to do the copy before reading it in CUDA.
     * It's not necessary if you only write into the texture  in CUDA, or if the
     * texture has not been modify in Unity. Tips : not necessary for write only
     * in CUDA or read only in Unity
     */
    UNITY_INTERFACE_EXPORT virtual int copyUnityTextureToAPITexture() = 0;

    /**
     * For some API (DX11) CUDA cannot edit the texture created by Unity
     * therefore, we have to create a new texture that will used as a buffer
     * between the unity texture and the surface object that is modify by CUDA
     * For these API, this function will copy the content of the buffer texture
     * to the unity texture, for the other API. It'll do nothing.
     * If API texture has been modify by, you have to do the copy before
     * reading it in Unity. It's not necessary if you only read into the
     * texture in CUDA, or if the texture is only write only in Unity. Tips :
     * not necessary for read only in CUDA or write only in Unity
     */
    UNITY_INTERFACE_EXPORT virtual int copyAPITextureToUnityTexture() = 0;

    /**
     * Map the cuda array from the graphics resources and create a surface
     * object from it. To write into the surface object use the getter of
     * _surfObjArray.
     */
    UNITY_INTERFACE_EXPORT int mapTextureToSurfaceObject(bool uploadToDevice = true);

    /**
     * Unmap the cuda array from graphics resources and destroy surface object
     * This function will wait for all previous GPU activity to complete
     */
    UNITY_INTERFACE_EXPORT int unmapTextureToSurfaceObject();

    /**
     * Generate the mips maps of the texture
     * For DX11 it doesn't works for Texture2D
     * see issue #6 https://github.com/davidAlgis/InteropUnityCUDA/issues/6
     */
    UNITY_INTERFACE_EXPORT virtual int generateMips() = 0;


    /**
     * Get the default dimension block (8,8,1)
     */
    UNITY_INTERFACE_EXPORT dim3 getDimBlock() const;

    /**
     * Get the default dimension grid ((sizeBuffer + 7)/8,((sizeBuffer +
     * 7)/8,1)
     */
    UNITY_INTERFACE_EXPORT dim3 getDimGrid() const;

    /**
     * Get the width of the texture
     */
    UNITY_INTERFACE_EXPORT int getWidth() const;

    /**
     * Get the height of the texture
     */
    UNITY_INTERFACE_EXPORT int getHeight() const;

    /**
     * Get the depth of the texture
     */
    UNITY_INTERFACE_EXPORT int getDepth() const;

    /**
     * Get the native texture pointer
     */
    UNITY_INTERFACE_EXPORT void *getNativeTexturePtr() const;

    
    UNITY_INTERFACE_EXPORT cudaGraphicsResource *getCudaGraphicsResource() const;

    /**
     * Get the pointer of d_surfObjArray
     * This array of surface object is necessary
     * to write or read into a texture
     */
    UNITY_INTERFACE_EXPORT cudaSurfaceObject_t *getSurfaceObjectArray() const;

    /**
     * Get the surface object associated to the given indexInArray
     * This array of surface object is necessary
     * to write or read into a texture
     * @param  indexInArray index of the texture to get the surface object (must
     * be between 0 and textureDepth)
     * @return             the surface object associated to it
     */
    UNITY_INTERFACE_EXPORT cudaSurfaceObject_t
    getSurfaceObject(int indexInArray = 0) const;


    const UNITY_INTERFACE_EXPORT cudaSurfaceObject_t *
    getSurfaceObjectVectorDataPointer() const;

    protected:
    // Pointer to the texture created in Unity
    void *_textureHandle;
    // width of the texture
    int _textureWidth;
    // height of the texture
    int _textureHeight;
    // depth of the texture <2 <=> to Texture2D; >1 <=> Texture2DArray
    int _textureDepth;
    // Resource that can be used to retrieve the surface object for CUDA
    cudaGraphicsResource *_graphicsResource;

    private:
    // An array of surface object that will be of the size of texture depth
    // This array is allocate on host side and will be copy to device memory
    // when texture is map to it
    std::vector<cudaSurfaceObject_t> surfObjVector{};
    cudaSurfaceObject_t *_surfObjArray;

    // A device array of surface object that will be of the size of texture depth
    // This array is allocate on device memory.
    // the surface object is the object that you can used to write into texture
    // from cuda api (eg. with surf2DWrite)
    cudaSurfaceObject_t *d_surfObjArray;

    dim3 _dimBlock;
    dim3 _dimGrid;
};
