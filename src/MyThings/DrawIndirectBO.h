#pragma once

#include "Buffer.h"

template <GLenum USAGE>
class DrawIndirectBO : public Buffer<GL_DRAW_INDIRECT_BUFFER, USAGE>
{
public:
    template <typename IndirectCommandType>
    inline void generateCommands(const IndirectCommandType* data, size_t commandsCount)
    {
        Buffer<GL_DRAW_INDIRECT_BUFFER, USAGE>::generate(data, commandsCount * sizeof(IndirectCommandType));
    }
};

using StaticDrawIndirectBO = DrawIndirectBO<GL_STATIC_DRAW>;
using DynamicDrawIndirectBO = DrawIndirectBO<GL_DYNAMIC_DRAW>;