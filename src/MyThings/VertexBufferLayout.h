#pragma once

#include <vector>

#include "glcore.h"

class VertexBufferLayout
{
public:
    struct Element
    {
        GLint count;
        GLenum type;
        GLboolean normalized;
        const void* offset;
    };

public:
    inline constexpr VertexBufferLayout()
    {

    }

    inline ~VertexBufferLayout()
    {

    }

    inline constexpr void pushEmptyBytes(GLuint count)
    {
        m_Stride += count;
    }

    inline constexpr void pushFloats(GLuint count, bool normalized = false)
    {
        auto& element = m_Elements.emplace_back();
        element.count = count;
        element.type = GL_FLOAT;
        element.normalized = normalized ? GL_TRUE : GL_FALSE;
        element.offset = reinterpret_cast<const void*>(count);

        m_Stride += sizeof(GLfloat) * count;
    }

    inline constexpr void pushInts(GLuint count, bool normalized = false)
    {
        auto& element = m_Elements.emplace_back();
        element.count = count;
        element.type = GL_INT;
        element.normalized = normalized ? GL_TRUE : GL_FALSE;
        element.offset = reinterpret_cast<const void*>(count);

        m_Stride += sizeof(GLint) * count;
    }

    inline constexpr void pushUnsignedInts(GLuint count, bool normalized = false)
    {
        auto& element = m_Elements.emplace_back();
        element.count = count;
        element.type = GL_UNSIGNED_INT;
        element.normalized = normalized ? GL_TRUE : GL_FALSE;
        element.offset = reinterpret_cast<const void*>(count);

        m_Stride += sizeof(GLuint) * count;
    }

    inline constexpr void pushUnsignedBytes(GLuint count, bool normalized = false)
    {
        auto& element = m_Elements.emplace_back();
        element.count = count;
        element.type = GL_UNSIGNED_BYTE;
        element.normalized = normalized ? GL_TRUE : GL_FALSE;
        element.offset = reinterpret_cast<const void*>(count);

        m_Stride += sizeof(GLubyte) * count;
    }

    inline constexpr const std::vector<Element>& getElements() const { return m_Elements; }

    inline constexpr GLuint getStride() const { return m_Stride; }
    
private:
    GLuint m_Stride = 0;
    std::vector<Element> m_Elements;
};