#pragma once

#include "glcore.h"

class ShaderStorageBufferObject
{
public:
    enum class AccessType
    {
        READ_ONLY,
        WRITE_ONLY,
        READ_WRITE
    };
public:
    inline constexpr ShaderStorageBufferObject()
    {}

    ShaderStorageBufferObject(const ShaderStorageBufferObject&) = delete;
    ShaderStorageBufferObject& operator=(const ShaderStorageBufferObject&) = delete;

    inline constexpr ShaderStorageBufferObject(ShaderStorageBufferObject&& other)
        : m_RendererID(other.m_RendererID), m_BindingPoint(other.m_BindingPoint)
    {
        other.m_RendererID = 0;
    }

    inline constexpr ShaderStorageBufferObject& operator=(ShaderStorageBufferObject&& other)
    {
        if (this != &other)
        {
            this->~ShaderStorageBufferObject();
            m_RendererID = other.m_RendererID;
            m_BindingPoint = other.m_BindingPoint;
            other.m_RendererID = 0;
        }

        return *this;
    }

    inline ~ShaderStorageBufferObject()
    {
        release();
    }

    inline void generate()
    {
        glGenBuffers(1, &m_RendererID);
    }

    inline void release()
    {
        if (m_RendererID != 0)
        {
            glDeleteBuffers(1, &m_RendererID);
            m_RendererID = 0;
            m_BindingPoint = 0;
            m_HasBindingPoint = false;
        }
    }

    inline void bind() const
    {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
    }

    inline void setData(const void* data, int size)
    {
        bind();
        glBufferData(GL_SHADER_STORAGE_BUFFER, size, data, GL_DYNAMIC_DRAW);
    }

    inline void setSubData(const void* data, int size, int offset)
    {
        bind();
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);
    }

    inline void clear()
    {
        setData(nullptr, 0);
    }

    inline void setBindingPoint(unsigned int bindingPoint)
    {
        m_BindingPoint = bindingPoint;
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_BindingPoint, m_RendererID);
        m_HasBindingPoint = true;
    }

    inline void* mapBuffer(AccessType accessType)
    {
        bind();
        void* ptr = glMapBuffer(GL_SHADER_STORAGE_BUFFER, AccessTypeToGL(accessType));
        return ptr;
    }

    inline void unmapBuffer()
    {
        bind();
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }

    inline constexpr GLuint getRendererID() const
    {
        return m_RendererID;
    }

    inline constexpr unsigned int getBindingPoint() const
    {
        return m_BindingPoint;
    }

    inline constexpr bool hasBindingPoint() const { return m_HasBindingPoint; }

private:
    inline constexpr static GLenum AccessTypeToGL(AccessType accessType)
    {
        switch (accessType)
        {
        case AccessType::READ_ONLY:
            return GL_READ_ONLY;
        case AccessType::WRITE_ONLY:
            return GL_WRITE_ONLY;
        case AccessType::READ_WRITE:
            return GL_READ_WRITE;
        default:
            return GL_READ_ONLY;
        }
    }

private:
    GLuint m_RendererID = 0;
    bool m_HasBindingPoint = false;
    unsigned int m_BindingPoint = 0;
};