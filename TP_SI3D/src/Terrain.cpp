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

    glBindBufferBase(GL_UNIFORM_BUFFER, block, m_InstancesData);
}

Terrain::Terrain()
    : Terrain(TerrainSpecs())
{}

Terrain::~Terrain()
{
    release_program(m_CubeShaderID);
    glDeleteTextures(1, &m_SpriteSheetTextureID);
}

void Terrain::loadData()
{
    m_HeightMap = read_image("data/terrain/terrain.png");

    m_SpriteSheetTextureID = read_texture(0, "data/CubeWorld/Blocks_PixelArt.png");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    m_GrassBash.loadMesh("data/CubeWorld/Pixel Blocks/Grass.obj");
    
    m_CubeShaderID = read_program("data/shaders/TP_SI3D/Cube.glsl");
    program_print_errors(m_CubeShaderID);
}

void Terrain::initCubeTransforms()
{
    constexpr float CUBE_INITIAL_SIZE = 2.f;
    m_CubeTransforms.reserve(m_Specs.cubesWidth * m_Specs.cubesWidth);

    for (int i = 0; i < m_Specs.cubesWidth; i++)
    {
        for (int j = 0; j < m_Specs.cubesWidth; j++)
        {
            const float normalizedX = static_cast<float>(i) / m_Specs.cubesWidth;
            const float normalizedZ = static_cast<float>(j) / m_Specs.cubesWidth;
            const float normalizedY = m_HeightMap.texture(normalizedX, normalizedZ).r;

            const float X = (static_cast<float>(i)) * m_Specs.cubeSize * CUBE_INITIAL_SIZE;
            const float Y = std::floor(normalizedY * static_cast<float>(m_Specs.cubesHeight)) * m_Specs.cubeSize * CUBE_INITIAL_SIZE;
            const float Z = (static_cast<float>(j)) * m_Specs.cubeSize * CUBE_INITIAL_SIZE;

            m_CubeTransforms.push_back(Transpose(Translation(X, Y, Z) * Scale(m_Specs.cubeSize)));
        }
    }

    glGenBuffers(1, &m_InstancesData);
    glBindBuffer(GL_UNIFORM_BUFFER, m_InstancesData);
    glBufferData(GL_UNIFORM_BUFFER, m_CubeTransforms.size() * sizeof(Transform), m_CubeTransforms.data(), GL_STATIC_DRAW);
}

void Terrain::draw(const Transform& view, const Transform& projection) const
{
    glUseProgram(m_CubeShaderID);

    glUniform1i(glGetUniformLocation(m_CubeShaderID, "spriteSheet"), 0);
    glBindTexture(GL_TEXTURE_2D, m_SpriteSheetTextureID);

    glUniformMatrix4fv(glGetUniformLocation(m_CubeShaderID, "projectionMatrix"), 1, GL_TRUE, projection.data());
    glUniformMatrix4fv(glGetUniformLocation(m_CubeShaderID, "viewMatrix"), 1, GL_TRUE, view.data());

    m_GrassBash.draw();
}