#pragma once

#include <vector>

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
        m_InstanceTransforms.clear();
        m_InstanceTransforms.reserve(chunkWidth * chunkWidth);

        for (size_t i = startX; i < startX + chunkWidth; i++)
        {
            for (size_t j = startY; j < startY + chunkWidth; j++)
            {
                const float normalizedX = static_cast<float>(i) / totalX;
                const float normalizedZ = static_cast<float>(j) / totalY;
                const float normalizedY = heightMap.texture(normalizedX, normalizedZ).r;

                // @todo I don't think the desired size is matched with these formulas.
                const float X = (static_cast<float>(i)) * cubeInitialSize;
                const float Y = std::floor(normalizedY * static_cast<float>(maxHeight)) * cubeInitialSize;
                const float Z = (static_cast<float>(j)) * cubeInitialSize;

                m_InstanceTransforms.emplace_back(Transpose(Scale(cubeDesiredSize / cubeInitialSize) * Translation(X, Y, Z)));

                // std::cout << "Instance " << i << ", " << j << " created\n";
            }
        }

        m_BoundingBox.first = { startX * cubeDesiredSize, 0.f, startY * cubeDesiredSize };
        m_BoundingBox.size = { chunkWidth * cubeDesiredSize, maxHeight * cubeDesiredSize, chunkWidth * cubeDesiredSize };
    }

    inline const auto& getInstanceTransforms() const { return m_InstanceTransforms; }

    inline size_t getInstanceCount() const { return m_InstanceTransforms.size(); }

    inline const AABB& getboundingBox() const { return m_BoundingBox; }

private:
    std::vector<Transform> m_InstanceTransforms;
    AABB m_BoundingBox;
};