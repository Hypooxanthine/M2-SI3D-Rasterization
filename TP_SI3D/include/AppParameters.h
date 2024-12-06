#pragma once

#include <array>
#include <string_view>

#define LOG_SHOWN_CHUNKS 1

inline constexpr std::array<std::string_view, 2> MESHES = {
    "data/CubeWorld/Pixel Blocks/OBJ/Dirt.obj",
    "data/CubeWorld/Pixel Blocks/OBJ/Grass.obj"
};

inline constexpr auto MESH_COUNT = MESHES.size();

inline constexpr size_t DIRT_ID = 0, GRASS_ID = 1;