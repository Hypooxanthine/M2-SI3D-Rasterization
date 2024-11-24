#pragma once

#include "program.h"

#include "Shader.h"

class ComputeShader : public Shader
{
public:
    inline constexpr ComputeShader() = default;

    inline ~ComputeShader() = default;

    inline void dispatch(GLuint groupsX, GLuint groupsY, GLuint groupsZ) const
    {
        bind();
        glDispatchCompute(groupsX, groupsY, groupsZ);
    }

    inline void dispatch(GLuint localSizeX, GLuint localSizeY, GLuint localSizeZ,
                         GLuint minGlobalX, GLuint minGlobalY, GLuint minGlobalZ)
    {
        GLuint groupsX = (minGlobalX - 1) / localSizeX + 1;
        GLuint groupsY = (minGlobalY - 1) / localSizeY + 1;
        GLuint groupsZ = (minGlobalZ - 1) / localSizeZ + 1;

        dispatch(groupsX, groupsY, groupsZ);
    }
};