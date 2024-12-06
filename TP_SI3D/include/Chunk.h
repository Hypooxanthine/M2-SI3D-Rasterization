#pragma once

#include <vector>
#include <map>

#include "AppParameters.h"
#include "mat.h"
#include "image.h"
#include "FrustumCulling.h"

class Chunk
{
public:
    inline Chunk() = default;

    inline ~Chunk()
    {

    }

    inline void initInstanceTransforms(const Image& heightMap, size_t chunkWidth, size_t startX, size_t startY, size_t totalX, size_t totalY, size_t maxHeight, float cubeInitialSize, float cubeDesiredSize)
    {
        m_InstanceCount = 0;
        m_TransformsPerMesh.clear();
        for (size_t i = 0; i < MESH_COUNT; ++i)
            m_TransformsPerMesh[i] = {};

        for (size_t i = startX; i < startX + chunkWidth; i++)
        {
            for (size_t j = startY; j < startY + chunkWidth; j++)
            {
                const float normalizedX = static_cast<float>(i) / totalX;
                const float normalizedZ = static_cast<float>(j) / totalY;
                const float normalizedY = heightMap.texture(normalizedX, normalizedZ).r;

                const float X = (static_cast<float>(i)) * cubeInitialSize;
                const float Y = std::floor(normalizedY * static_cast<float>(maxHeight)) * cubeInitialSize;
                const float Z = (static_cast<float>(j)) * cubeInitialSize;

                size_t meshId;

                if (normalizedY > .9f) meshId = ICE_ID;
                else if (normalizedY > .3f) meshId = SNOW_ID;
                else meshId = GRASS_ID;

                m_TransformsPerMesh.at(meshId).emplace_back(
                    Transpose(Scale(cubeDesiredSize / cubeInitialSize) * Translation(X, Y, Z))
                );
                ++m_InstanceCount;

                // std::cout << "Instance " << i << ", " << j << " created\n";
            }
        }

        m_BoundingBox.first = { startX * cubeDesiredSize, 0.f, startY * cubeDesiredSize };
        m_BoundingBox.second = { (startX + chunkWidth) * cubeDesiredSize, maxHeight * cubeDesiredSize, (startY + chunkWidth) * cubeDesiredSize };
    }

    inline constexpr size_t getInstanceCount() const { return m_InstanceCount; }

    inline const auto& getMeshTransforms() const { return m_TransformsPerMesh; }

    inline const AABB& getboundingBox() const { return m_BoundingBox; }

private:
    std::map<size_t, std::vector<Transform>> m_TransformsPerMesh;
    size_t m_InstanceCount;
    AABB m_BoundingBox;
};