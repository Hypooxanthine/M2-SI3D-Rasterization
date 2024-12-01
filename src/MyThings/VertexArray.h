#pragma once

#include "glcore.h"
#include "VertexBufferLayout.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"

class VertexArray
{
public:
    inline constexpr VertexArray()
    {
    }

    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(const VertexArray&) = delete;

    inline constexpr VertexArray& operator=(VertexArray&& other) noexcept
    {
        if (this != &other)
        {
            m_RenderID = std::move(other.m_RenderID);
            other.m_RenderID = 0;
        }

        return *this;
    }

    inline constexpr VertexArray(VertexArray&& other) noexcept
    {
        *this = std::move(other);
    }

    inline ~VertexArray()
    {
        releaseGpuMemory();
    }

    inline void generate(const VertexBufferLayout& layout)
    {
        releaseGpuMemory();
        glGenVertexArrays(1, &m_RenderID);
    }

    template <GLenum VboUsage>
    inline void addLayout(const VertexBuffer<VboUsage>& vbo, const VertexBufferLayout& layout)
    {
        m_AttributeCount += setLayout(m_AttributeCount, vbo, layout);
    }
    
    /**
     * Modifie les attributs à partir de attributeIndex pour les faire correspondre au layout.
     * Retourne le nombre d'attributs modifiés. S'ils n'existent pas, ils sont créés.
     */
    template <GLenum VboUsage>
    inline GLuint setLayout(GLuint attributeIndex, const VertexBuffer<VboUsage>& vbo, const VertexBufferLayout& layout)
    {
        bind();
        vbo.bind();
        
        GLuint i = 0;

        for (const auto& element : layout.getElements())
        {
            glEnableVertexAttribArray(attributeIndex + i);
            glVertexAttribPointer(
                attributeIndex + i,
                element.count,
                element.type,
                element.normalized,
                layout.getStride(),
                element.offset
            );

            ++i;
        }

        return i;
    }

    template <GLenum IboUsage>
    inline void setIndexBuffer(const IndexBuffer<IboUsage>& ibo)
    {
        bind();
        ibo.bind();
    }

    inline static void bind(GLuint renderId)
    {
        if (renderId == s_CurrentRenderID)
            return;

        glBindVertexArray(renderId);
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
        m_AttributeCount = 0;
    }

    inline constexpr GLuint getRenderId() const { return m_RenderID; }

private:
    inline void releaseGpuMemory() const
    {
        if (m_RenderID != 0)
            glDeleteVertexArrays(1, &m_RenderID);
    }

private:
    GLuint m_RenderID = 0;
    GLuint m_AttributeCount = 0;
    inline static GLuint s_CurrentRenderID = 0;
};