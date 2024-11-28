#pragma once

#include "glcore.h"
#include "texture.h"

class Texture3D
{
public:
    inline constexpr Texture3D()
    {}

    Texture3D& operator=(const Texture3D&) = delete;

    Texture3D(const Texture3D& other) = delete;

    inline Texture3D& operator=(Texture3D&& other)
    {
        release();
        m_RenderID = std::move(other.m_RenderID);
        m_Width = std::move(other.m_Width);
        m_Height = std::move(other.m_Height);
        m_Layers = std::move(other.m_Layers);

        other.m_RenderID = 0;
        other.m_Width = 0;
        other.m_Height = 0;
        other.m_Layers = 0;

        return *this;
    }

    inline Texture3D(Texture3D&& other)
    {
        *this = std::move(other);
    }

    inline ~Texture3D()
    {
        release();
    }

    inline void createForColors(size_t width, size_t height, size_t layers)
    {
        release();

        glGenTextures(1, &m_RenderID);
        bind();
        glTexStorage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, width, height, layers);

        m_Width = width;
        m_Height = height;
        m_Layers = layers;
    }

    inline void setlayer(size_t layer, const ImageData& image)
    {
        bind();

        GLenum format;
        switch(image.channels)
        {
            case 1: format= GL_RED; break;
            case 2: format= GL_RG; break;
            case 3: format= GL_RGB; break;
            case 4: format= GL_RGBA; break;
            default: format= GL_RGBA; 
        }
    
        GLenum type;
        switch(image.size)
        {
            case 1: type= GL_UNSIGNED_BYTE; break;
            case 4: type= GL_FLOAT; break;
            default: type= GL_UNSIGNED_BYTE;
        }

        glTexSubImage3D(
            GL_TEXTURE_2D_ARRAY,
            0, 0, 0, layer,
            m_Width, m_Height, m_Layers,
            format, type,
            image.data()
        );

        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D_ARRAY,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);

        // glGenerateMipmap(GL_TEXTURE_2D_ARRAY);
    }

    inline void bind()
    {
        glBindTexture(GL_TEXTURE_2D_ARRAY, m_RenderID);
    }

    inline void release()
    {
        if (m_RenderID != 0)
        {
            glDeleteTextures(1, &m_RenderID);
            m_RenderID = 0;
            m_Width = 0;
            m_Height = 0;
            m_Layers = 0;
        }
    }

    inline constexpr GLuint getRenderId() const { return m_RenderID; }

    inline constexpr size_t getWidth() const { return m_Width; }

    inline constexpr size_t getHeight() const { return m_Height; }

private:
    GLuint m_RenderID = 0;
    size_t m_Width = 0, m_Height = 0, m_Layers = 0;
};