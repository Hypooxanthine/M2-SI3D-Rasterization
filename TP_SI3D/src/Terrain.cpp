#include "Terrain.h"

#include <wavefront.h>
#include <image_io.h>

Terrain::Terrain(const TerrainSpecs& specs)
    : m_Specs(specs)
{
    loadData();
    initCubeTransforms();
}

Terrain::Terrain()
    : Terrain(TerrainSpecs())
{}

Terrain::~Terrain()
{
}

void Terrain::loadData()
{
    m_HeightMap = read_image("data/terrain/terrain.png");

    auto imageData = read_image_data("data/CubeWorld/Blocks_PixelArt.png");
    ASSERT(imageData.size > 0, "Could not load data/CubeWorld/Blocks_PixelArt.png");
    ASSERT(m_SpriteSheetTexture.loadFromImage(imageData), "Could not load spritesheet from image data");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    ASSERT(m_CubeShader.load("data/shaders/TP_SI3D/Cube.glsl"), "Could not load data/shaders/TP_SI3D/Cube.glsl");

    m_Meshes.emplace_back(read_indexed_mesh("data/CubeWorld/Pixel Blocks/OBJ/Dirt.obj"));
    m_Meshes.emplace_back(read_indexed_mesh("data/CubeWorld/Pixel Blocks/OBJ/Grass.obj"));
    
    for (const auto& mesh : m_Meshes)
        m_MultiMesh.addMesh(mesh);
    
    auto instanceCount = m_Specs.cubesWidth * m_Specs.cubesWidth;
    m_MultiMesh.setMeshInstanceCount(0, instanceCount / 2);
    m_MultiMesh.setMeshInstanceCount(1, instanceCount - instanceCount / 2);
    m_MultiMesh.createBuffers();
}

void Terrain::initCubeTransforms()
{
    constexpr float CUBE_INITIAL_SIZE = 2.f;
    m_InstanceTransforms.reserve(m_Specs.cubesWidth * m_Specs.cubesWidth);

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

            m_InstanceTransforms.emplace_back(Transpose(Translation(X, Y, Z) * Scale(m_Specs.cubeSize)));
        }
    }

    m_SSBO.generate();
    m_SSBO.setBindingPoint(0);
    m_SSBO.setData(m_InstanceTransforms.data(), m_InstanceTransforms.size());
}

void Terrain::draw(const Transform& view, const Transform& projection) const
{
    // m_CubeShader.bind();

    // m_CubeShader.setTextureUniform(m_SpriteSheetTexture, 0, "spriteSheet");

    // m_CubeShader.setUniform("projectionMatrix", projection);
    // m_CubeShader.setUniform("viewMatrix", view);

    // m_GrassBash.draw();
    m_CubeShader.bind();
    m_CubeShader.setUniform("viewMatrix", view);
    m_CubeShader.setUniform("projectionMatrix", projection);
    m_CubeShader.setTextureUniform(m_SpriteSheetTexture, 0, "spriteSheet");

    m_MultiMesh.draw();
}