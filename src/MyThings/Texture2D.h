#pragma once

#include <utility>

#include "glcore.h"
#include "texture.h"

#include "Assert.h"

class Texture2D
{
public:
    inline constexpr Texture2D()
    {}

    Texture2D(const Texture2D& other) = delete;

    inline Texture2D& operator=(Texture2D&& other)
    {
        release();
        m_RenderID = std::move(other.m_RenderID);
        m_Width = std::move(other.m_Width);
        m_Height = std::move(other.m_Height);

        other.m_RenderID = 0;
        other.m_Width = 0;
        other.m_Height = 0;

        return *this;
    }

    inline Texture2D(Texture2D&& other)
    {
        *this = std::move(other);
    }

    inline ~Texture2D()
    {
        release();
    }

    inline bool loadFromImage(const ImageData& image)
    {
        release();

        m_RenderID = make_texture(0, image);
        if (m_RenderID == 0) return false;
        
        m_Width = image.width;
        m_Height = image.height;

        return true;
    }

    inline void generateForDepth(size_t width, size_t height)
    {
        release();

        m_Width = width;
        m_Height = height;

        m_RenderID = make_depth_texture(0, m_Width, m_Height);
    }

    inline void generateForColor(size_t width, size_t height, bool useAlpha)
    {
        release();

        m_Width = width;
        m_Height = height;

        if (useAlpha)
            m_RenderID = make_vec4_texture(0, m_Width, m_Height);
        else
            m_RenderID = make_vec3_texture(0, m_Width, m_Height);
    }

    inline void bind()
    {
        glBindTexture(GL_TEXTURE_2D, m_RenderID);
    }

    inline void release()
    {
        if (m_RenderID != 0)
        {
            glDeleteTextures(1, &m_RenderID);
            m_RenderID = 0;
            m_Width = 0;
            m_Height = 0;
        }
    }

    inline constexpr GLuint getRenderId() const { return m_RenderID; }

    inline constexpr size_t getWidth() const { return m_Width; }

    inline constexpr size_t getHeight() const { return m_Height; }

private:
    GLuint m_RenderID = 0;
    size_t m_Width = 0, m_Height = 0;
};