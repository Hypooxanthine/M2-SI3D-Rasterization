#pragma once

#include "glcore.h"

template <GLenum TARGET, GLenum USAGE>
class Buffer
{
public:
    inline constexpr Buffer()
    {
    }

    Buffer& operator=(const Buffer&) = delete;
    Buffer(const Buffer&) = delete;

    inline constexpr Buffer& operator=(Buffer&& other) noexcept
    {
        if (this != &other)
        {
            m_RenderID = std::move(other.m_RenderID);
            other.m_RenderID = 0;
        }

        return *this;
    }

    inline constexpr Buffer(Buffer&& other) noexcept
    {
        *this = std::move(other);
    }

    inline ~Buffer()
    {
        releaseGpuMemory();
    }

    inline void generate(const void* data, size_t dataSize)
    {
        releaseGpuMemory();

        glGenBuffers(1, &m_RenderID);
        bind();
        glBufferData(TARGET, dataSize, data, GL_STATIC_DRAW);
    }

    inline static void bind(GLuint renderId)
    {
        if (renderId == s_CurrentRenderID)
            return;

        glBindBuffer(TARGET, renderId);
        s_CurrentRenderID = renderId;
    }

    inline void bind() const
    {
        bind(m_RenderID);
    }

    inline static void unbind()
    {
        bind(0);
    }

    inline void release()
    {
        releaseGpuMemory();
        m_RenderID = 0;
    }

    inline constexpr GLuint getRenderId() const { return m_RenderID; }

private:
    inline void releaseGpuMemory()
    {
        if (m_RenderID != 0)
            glDeleteBuffers(1, &m_RenderID);
    }

private:
    GLuint m_RenderID = 0;
    inline static GLuint s_CurrentRenderID = 0;
};