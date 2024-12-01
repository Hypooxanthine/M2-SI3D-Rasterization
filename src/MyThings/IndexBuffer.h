#pragma once

#include "Buffer.h"

template <GLenum USAGE>
class IndexBuffer : public Buffer<GL_ELEMENT_ARRAY_BUFFER, USAGE>
{
public:
    template <typename IndexType>
    inline void generateIndices(const IndexType* data, size_t indexCount)
    {
        Buffer<GL_ELEMENT_ARRAY_BUFFER, USAGE>::generate(data, indexCount * sizeof(IndexType));
    }
};

using StaticIndexBuffer = IndexBuffer<GL_STATIC_DRAW>;
using DynamicIndexBuffer = IndexBuffer<GL_DYNAMIC_DRAW>;