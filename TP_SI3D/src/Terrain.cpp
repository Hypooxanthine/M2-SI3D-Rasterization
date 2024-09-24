#include "Terrain.h"

#include <wavefront.h>
#include <image_io.h>

Terrain::Terrain(const TerrainSpecs& specs)
    : m_Specs(specs)
{
    loadData();
    initCubeTransforms();

    GLuint block = glGetUniformBlockIndex(m_CubeShaderID, "instanceData");
    glUniformBlockBinding(m_CubeShaderID, block, 0);

    glGenBuffers(1, &m_InstanceData);
    glBindBuffer(GL_UNIFORM_BUFFER, m_InstanceData);
    glBufferData(GL_UNIFORM_BUFFER, m_CubeTransforms.size() * sizeof(Transform), nullptr, GL_STATIC_DRAW);
}

Terrain::Terrain()
    : Terrain(TerrainSpecs())
{}

void Terrain::loadData()
{
    m_CubeMesh = read_indexed_mesh("data/cube.obj");
    m_HeightMap = read_image("data/terrain/terrain.png");
    m_CubeShaderID = read_program("data/shaders/TP_SI3D/Cube.glsl");
    program_print_errors(m_CubeShaderID);
}

void Terrain::initCubeTransforms()
{
    for (int i = 0; i < m_HeightMap.width(); i++)
    {
        for (int j = 0; j < m_HeightMap.height(); j++)
        {
            const float normalizedX = static_cast<float>(i) / m_HeightMap.width();
            const float normalizedZ = -static_cast<float>(j) / m_HeightMap.height();
            const float normalizedY = m_HeightMap(i, j).r;
            const int cubeX = normalizedX * m_Specs.cubesWidth;
            const int cubeZ = normalizedZ * m_Specs.cubesWidth;
            const int cubeY = normalizedY * m_Specs.cubesHeight;

            const float X = cubeX * m_Specs.cubeSize;
            const float Y = cubeY * m_Specs.cubeSize;
            const float Z = cubeZ * m_Specs.cubeSize;

            m_CubeTransforms.emplace_back(X, Y, Z);
        }
    }
}

void Terrain::draw(const Transform& view, const Transform& projection)
{
    glBindVertexArray(m_CubeMesh.create_buffers(true, true, false, true));
    glUseProgram(m_CubeShaderID);
    glUniformMatrix4fv(glGetUniformLocation(m_CubeShaderID, "projectionMatrix"), 1, GL_TRUE, projection.data());
    glUniformMatrix4fv(glGetUniformLocation(m_CubeShaderID, "viewMatrix"), 1, GL_TRUE, view.data());

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_InstanceData);

    glDrawElementsInstanced(GL_TRIANGLES, m_CubeMesh.vertex_count(), GL_UNSIGNED_INT, nullptr, m_CubeTransforms.size());
}