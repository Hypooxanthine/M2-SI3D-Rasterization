#pragma once

#include "Buffer.h"

template <GLenum USAGE>
class VertexBuffer : public Buffer<GL_ARRAY_BUFFER, USAGE>
{
public:
    template <typename VertexType>
    inline void generateVertices(const VertexType* data, size_t vertexCount)
    {
        Buffer<GL_ARRAY_BUFFER, USAGE>::generate(data, vertexCount * sizeof(VertexType));
    }
};

using StaticVertexBuffer = VertexBuffer<GL_STATIC_DRAW>;
using DynamicVertexBuffer = VertexBuffer<GL_DYNAMIC_DRAW>;
