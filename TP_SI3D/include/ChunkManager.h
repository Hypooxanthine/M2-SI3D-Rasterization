#pragma once

#include "Chunk.h"

class ChunkManager
{
public:
    inline ChunkManager() = default;

    inline void init(size_t chunksX, size_t chunksY, size_t chunkWidth, const Image& heightMap, size_t maxHeight, float cubeInitialSize, float cubeDesiredSize)
    {
        m_Chunks.clear();
        m_ChunksFirstInstanceIndex.clear();
        m_Chunks.reserve(chunksX * chunksY);
        m_ChunksFirstInstanceIndex.reserve(chunksX * chunksY);
        m_TotalInstanceCount = 0;

        for (size_t i = 0; i < chunksX; ++i)
        {
            for (size_t j = 0; j < chunksY; ++j)
            {
                auto& chunk = m_Chunks.emplace_back();
                chunk.initInstanceTransforms(heightMap, chunkWidth, i * chunkWidth, j * chunkWidth, chunksX * chunkWidth, chunksY * chunkWidth, maxHeight, cubeInitialSize, cubeDesiredSize);

                m_ChunksFirstInstanceIndex.emplace_back(m_TotalInstanceCount);
                m_TotalInstanceCount += chunk.getInstanceCount();
            }
        }
    }

    inline const auto& getChunks() const { return m_Chunks; }

    inline const auto& getChunkFirstInstanceIndice() const { return m_ChunksFirstInstanceIndex; }

    inline size_t getTotalInstanceCount() const { return m_TotalInstanceCount; }

private:
    std::vector<Chunk> m_Chunks;
    std::vector<size_t> m_ChunksFirstInstanceIndex;
    size_t m_TotalInstanceCount = 0;
};