#pragma once

#include <mesh.h>
#include <image.h>
#include <program.h>
#include <texture.h>

#include <vector>

#include "mat.h"
#include "Texture2D.h"
#include "Shader.h"
#include "MultiMesh.h"
#include "ShaderStorageBufferObject.h"

class Terrain
{
public:
    struct TerrainSpecs
    {
        int cubesWidth = 200;
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

    Shader m_CubeShader;

    std::vector<Mesh> m_Meshes;
    MultiMesh m_MultiMesh;
    std::vector<Transform> m_InstanceTransforms;
    ShaderStorageBufferObject m_SSBO;
};
