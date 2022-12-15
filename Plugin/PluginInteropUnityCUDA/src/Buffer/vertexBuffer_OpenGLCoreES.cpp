#pragma once
#include "vertexBuffer_OpenGLCoreES.h"
#include "openGLInclude.h"
#include <cuda_gl_interop.h>
// OpenGL Core profile (desktop) or OpenGL ES (mobile) implementation of RenderAPI.
// Supports several flavors: Core, ES2, ES3


#if SUPPORT_OPENGL_UNIFIED

VertexBuffer_OpenGLCoreES::VertexBuffer_OpenGLCoreES(void* bufferHandle, int size)
	: VertexBuffer(bufferHandle, size)
{}

VertexBuffer_OpenGLCoreES::~VertexBuffer_OpenGLCoreES()
{
	// final check to be sure there was no mistake
	GL_CHECK();
	CUDA_CHECK(cudaGetLastError());
};

/// <summary>
/// Register the buffer from OpenGL to CUDA
/// </summary>
void VertexBuffer_OpenGLCoreES::registerBufferInCUDA()
{
	// cast the pointer on the buffer of unity to gluint
	GLuint glBuffer = (GLuint)(_bufferHandle);
	//glBindBuffer(GL_PIXEL_UNPACK_BUFFER, glBuffer);
	GL_CHECK();
	//register the buffer to cuda : it initialize the _pGraphicsResource
	CUDA_CHECK(cudaGraphicsGLRegisterBuffer(&_pGraphicsResource, glBuffer, cudaGraphicsRegisterFlagsNone));
}

int VertexBuffer_OpenGLCoreES::SetTextureFromBuffer(Texture& texture) 
{
	//TODO: add some check on type
	if (texture.getHeight() * texture.getWidth() != _size) 
	{
		Log::log().debugLogError("Cannot create a texture from a buffer which has a different size");
		return -1;
	}

	// Select the appropriate buffer
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER, (GLuint)_bufferHandle);
	// Select the appropriate texture
	glBindTexture(GL_TEXTURE_2D, (GLuint)texture.getNativeTexturePtr());

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, texture.getWidth(), texture.getHeight(), GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void VertexBuffer_OpenGLCoreES::unRegisterBufferInCUDA()
{
	CUDA_CHECK(cudaGraphicsUnregisterResource(_pGraphicsResource));
}


#endif // #if SUPPORT_OPENGL_UNIFIED
