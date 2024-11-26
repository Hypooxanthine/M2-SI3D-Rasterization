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

    ASSERT(m_GrassBash.load("data/CubeWorld/Pixel Blocks/OBJ/Grass.obj"), "Could not load data/CubeWorld/Pixel Blocks/OBJ/Grass.obj");
    
    ASSERT(m_CubeShader.load("data/shaders/TP_SI3D/Cube.glsl"), "Could not load data/shaders/TP_SI3D/Cube.glsl");
}

void Terrain::initCubeTransforms()
{
    constexpr float CUBE_INITIAL_SIZE = 2.f;
    m_GrassBash.reserveInstances(m_Specs.cubesWidth * m_Specs.cubesWidth);

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

            m_GrassBash.pushInstance(Transpose(Translation(X, Y, Z) * Scale(m_Specs.cubeSize)));
            std::cout << "Cube position (X, Y, Z): (" << X << ", " << Y << ", " << Z << ")\n";
        }
    }

    m_GrassBash.setupUniformBuffer(m_CubeShader);
}

void Terrain::draw(const Transform& view, const Transform& projection) const
{
    m_CubeShader.bind();

    m_CubeShader.setTextureUniform(m_SpriteSheetTexture, 0, "spriteSheet");

    m_CubeShader.setUniform("projectionMatrix", projection);
    m_CubeShader.setUniform("viewMatrix", view);

    m_GrassBash.draw();
}