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

        for (size_t i = 0; i < chunksX; ++i)
        {
            for (size_t j = 0; j < chunksY; ++j)
            {
                auto& chunk = m_Chunks.emplace_back();
                chunk.initInstanceTransforms(heightMap, chunkWidth, i * chunkWidth, j * chunkWidth, chunksX * chunkWidth, chunksY * chunkWidth, maxHeight, cubeInitialSize, cubeDesiredSize);
                
                size_t lastChunkId = m_Chunks.size() - 1;
                size_t firstInstanceIndex = 0;
                if (lastChunkId > 0)
                    firstInstanceIndex = m_Chunks.at(lastChunkId - 1).getInstanceCount() + m_ChunksFirstInstanceIndex.back();

                m_ChunksFirstInstanceIndex.emplace_back(firstInstanceIndex);

                std::cout << "Chunk " << i << ", " << j << " created, with instance first index: " << firstInstanceIndex << "\n";
            }
        }
    }

    const auto& getChunks() const { return m_Chunks; }

    const auto& getChunkFirstInstanceIndice() const { return m_ChunksFirstInstanceIndex; }

private:
    std::vector<Chunk> m_Chunks;
    std::vector<size_t> m_ChunksFirstInstanceIndex;
};