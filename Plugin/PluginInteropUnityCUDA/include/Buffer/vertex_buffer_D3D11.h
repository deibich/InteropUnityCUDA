#pragma once

#include "vertex_buffer.h"

#if SUPPORT_D3D11

class VertexBuffer_D3D11 : public VertexBuffer
{
    public:
    VertexBuffer_D3D11(void *bufferHandle, int size);
    ~VertexBuffer_D3D11();
    virtual void registerBufferInCUDA();
    virtual void unRegisterBufferInCUDA();
};

#endif
