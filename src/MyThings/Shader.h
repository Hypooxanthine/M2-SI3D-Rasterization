#pragma once

#include <string_view>

#include "Texture2D.h"
#include "program.h"
#include "uniforms.h"

class Shader
{
public:
    inline constexpr Shader() {}

    inline virtual ~Shader()
    {
        release();
    }

    inline void generate()
    {
        m_RenderId = glCreateProgram();
    }

    inline void release()
    {
        if (m_RenderId != 0)
        {
            release_program(m_RenderId);
        }
    }

    inline bool load(const std::string_view& path)
    {
        if (m_RenderId == 0) generate();
        
        return reload(path);
    }

    inline bool reload(const std::string_view& path)
    {
        if (m_RenderId != 0)
        {
            reload_program(m_RenderId, path.data());
            return compiled();
        }
        else
        {
            std::cerr << "Couldn't reload shader: shader is not allocated.\n";
            return false;
        }
    }

    inline bool compiled(std::string& errorLogs) const
    {
        int code = program_format_errors(m_RenderId, errorLogs);
        return code == 0;
    }

    inline bool compiled(bool logOnFail = true) const
    {
        std::string logs;
        bool out = compiled(logs);

        if (logOnFail && !logs.empty())
        {
            std::cerr << logs << '\n';
        }

        return out;
    }

    inline void bind() const
    {
        glUseProgram(m_RenderId);
    }

    template <typename T>
    inline void setUniform(const std::string_view& name, const T& value) const
    {
        program_uniform(m_RenderId, name.data(), value);
    }

    inline void setTextureUniform(const Texture2D& texture, GLuint slot) const;

    inline constexpr GLuint getRenderId() const { return m_RenderId; }

private:
    GLuint m_RenderId = 0;
};

#include "Texture2D.h"

inline void Shader::setTextureUniform(const Texture2D& texture, GLuint slot) const
{    
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, texture.getRenderId());
}