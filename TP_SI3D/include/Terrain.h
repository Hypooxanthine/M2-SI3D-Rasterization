#pragma once

#include <mesh.h>
#include <image.h>
#include <program.h>
#include <texture.h>

#include <vector>

#include "CubeBash.h"
#include "Texture2D.h"
#include "Shader.h"

class Terrain
{
public:
    struct TerrainSpecs
    {
        int cubesWidth = 50;
        int cubesHeight = 20;
        float cubeSize = .01f;
    };
public:
    Terrain(const TerrainSpecs& specs);
    Terrain();
    ~Terrain();

    void draw(const Transform& view, const Transform& projection) const;

private:
    void loadData();
    void initCubeTransforms();

private:
    TerrainSpecs m_Specs;
    
    Image m_HeightMap;

    Texture2D m_SpriteSheetTexture;

    CubeBash m_GrassBash;

    Shader m_CubeShader;
};
