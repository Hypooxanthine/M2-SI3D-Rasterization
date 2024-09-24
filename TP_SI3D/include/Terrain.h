#pragma once

#include <mesh.h>
#include <image.h>
#include <program.h>

#include <vector>

class Terrain
{
public:
    struct TerrainSpecs
    {
        int cubesWidth = 100;
        int cubesHeight = 100;
        float cubeSize = 1.f;
    };
public:
    Terrain(const TerrainSpecs& specs);
    Terrain();

    void draw(const Transform& view, const Transform& projection);

private:
    void loadData();
    void initCubeTransforms();

private:
    TerrainSpecs m_Specs;
    
    Mesh m_CubeMesh;
    Image m_HeightMap;

    GLuint m_CubeShaderID;
    GLuint m_InstanceData;
    std::vector<Transform> m_CubeTransforms;
};
