#pragma once
#include "texture_OpenGLCoreES.h"
#include "openGLInclude.h"
#include <cuda_gl_interop.h>
// OpenGL Core profile (desktop) or OpenGL ES (mobile) implementation of RenderAPI.
// Supports several flavors: Core, ES2, ES3


#if SUPPORT_OPENGL_UNIFIED


Texture_OpenGLCoreES::Texture_OpenGLCoreES(void* textureHandle, int textureWidth, int textureHeight)
	: Texture(textureHandle, textureWidth, textureHeight)
{}

Texture_OpenGLCoreES::~Texture_OpenGLCoreES()
{
	GL_CHECK();
	CUDA_CHECK(cudaGetLastError());
};

/// <summary>
/// Has to be call after the first issue plugin event 
/// see. https://docs.unity3d.com/ScriptReference/GL.IssuePluginEvent.html 
/// register a graphics resources defined from the texture openGL
/// </summary>
void Texture_OpenGLCoreES::registerTextureInCUDA()
{
	// cast the pointer on the texture of unity to gluint
	GLuint gltex = (GLuint)(size_t)(_textureHandle);
	glBindTexture(GL_TEXTURE_2D, gltex);
	GL_CHECK();
	// register the texture to cuda : it initialize the _pGraphicsResource
	CUDA_CHECK(cudaGraphicsGLRegisterImage(&_pGraphicsResource, gltex, GL_TEXTURE_2D, cudaGraphicsRegisterFlagsWriteDiscard));	
}


void Texture_OpenGLCoreES::unRegisterTextureInCUDA()
{
	CUDA_CHECK(cudaGraphicsUnregisterResource(_pGraphicsResource));
}

#endif // #if SUPPORT_OPENGL_UNIFIED
