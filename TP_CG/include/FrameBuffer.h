#pragma once

#include <array>

#include "glcore.h"

#include "Texture2D.h"

namespace details
{

inline constexpr std::array<GLenum, 8> CreateColorAttachmentsArray()
{
    std::array<GLenum, 8> out = { 0 };
    for (size_t i = 0; i < 8; ++i)
        out[i] = GL_COLOR_ATTACHMENT0 + (int)i;

    return out;
}

}

class FrameBuffer
{
public:
    enum class TextureAttachment
    {
        Depth = 0,
        Color,
    };

public:
    inline constexpr FrameBuffer()
    {}

    inline ~FrameBuffer()
    {
        release();
    }

    inline constexpr FrameBuffer& operator=(FrameBuffer&& other)
    {
        m_RenderID = std::move(other.m_RenderID);
        m_ColorAttachmentsCount = other.m_ColorAttachmentsCount;

        other.m_RenderID = 0;
        other.m_ColorAttachmentsCount = 0;
    }

    inline constexpr FrameBuffer(FrameBuffer&& other)
    {
        *this = std::move(other);
    }

    inline constexpr void generate(bool onScreen = false)
    {
        release();

        if (!onScreen)
            glGenFramebuffers(1, &m_RenderID);
    }

    inline void bind() const
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RenderID);
    }

    inline void release()
    {
        if (m_RenderID != 0)
        {
            glDeleteFramebuffers(1, &m_RenderID);
            m_RenderID = 0;
            m_ColorAttachmentsCount = 0;
        }
    }

    /**
     * @brief L'ordre des bindings est le même que l'ordre des appels. Les color attachments
     * sont faits incrémentalement en partant de attachment0.
     */
    inline constexpr void attachTexture(const Texture2D& texture, TextureAttachment textureAttachment, GLint mipmapLevel)
    {
        bind();

        GLenum attachment = GL_COLOR_ATTACHMENT0;
        if (textureAttachment == TextureAttachment::Depth)
            attachment = GL_DEPTH_ATTACHMENT;
        else
        {
            ASSERT(m_ColorAttachmentsCount < 8, "Cannot use more than 8 color attachments");
            attachment += m_ColorAttachmentsCount;
            ++m_ColorAttachmentsCount;
        }

        glFramebufferTexture(GL_DRAW_FRAMEBUFFER, attachment, texture.getRenderId(), mipmapLevel);
    }

    inline constexpr void setupBindings() const
    {
        glDrawBuffers(m_ColorAttachmentsCount, s_ColorAttachmentsInOrder.data());
        ASSERT(glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE,
            "Framebuffer incomplete");
    }

    inline GLuint getRenderId() const { return m_RenderID; }

private:
    GLuint m_RenderID = 0;

    uint8_t m_ColorAttachmentsCount = 0;

    static constexpr std::array<GLenum, 8> s_ColorAttachmentsInOrder = details::CreateColorAttachmentsArray();
};
