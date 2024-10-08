#pragma once

#include <mesh.h>
#include <image.h>
#include <program.h>
#include <texture.h>

#include <vector>

#include "CubeBash.h"

class Terrain
{
public:
    struct TerrainSpecs
    {
        int cubesWidth = 200;
        int cubesHeight = 10;
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

    GLuint m_SpriteSheetTextureID;

    CubeBash m_GrassBash;

    GLuint m_CubeShaderID;
    GLuint m_InstancesData;
};
