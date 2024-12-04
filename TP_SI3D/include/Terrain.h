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
#include "ChunkManager.h"

class Terrain
{
public:
    struct TerrainSpecs
    {
        size_t chunkX = 50;
        size_t chunkY = 50;
        size_t chunkWidth = 16;
        size_t cubesHeight = 40;
        float cubeSize = .1f;
    };
public:
    Terrain(const TerrainSpecs& specs);
    Terrain();
    ~Terrain();

    void draw(const Transform& view, const Transform& projection) const;
    void cullChunks(const Transform& view, const Transform& projection);
    void stopCulling();

private:
    void loadData();
    void initChunks();

private:
    TerrainSpecs m_Specs;
    
    Image m_HeightMap;

    Texture2D m_SpriteSheetTexture;

    Shader m_CubeShader;

    std::vector<Mesh> m_Meshes;
    ChunkManager m_ChunkManager;
    MultiMesh m_MultiMesh;
    std::vector<Transform> m_InstanceTransforms;
    ShaderStorageBufferObject m_SSBO;
};
