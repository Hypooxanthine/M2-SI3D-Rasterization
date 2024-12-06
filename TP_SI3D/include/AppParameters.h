#pragma once

#include <array>
#include <string_view>

#define LOG_SHOWN_CHUNKS 1

inline constexpr std::array<std::string_view, 5> MESHES = {
    "data/CubeWorld/Pixel Blocks/OBJ/Dirt.obj",
    "data/CubeWorld/Pixel Blocks/OBJ/Grass.obj",
    "data/CubeWorld/Pixel Blocks/OBJ/Block_Blank.obj",
    "data/CubeWorld/Pixel Blocks/OBJ/Snow.obj",
    "data/CubeWorld/Pixel Blocks/OBJ/Ice.obj"
};

inline constexpr auto MESH_COUNT = MESHES.size();

inline constexpr size_t
    DIRT_ID = 0,
    GRASS_ID = 1,
    BLANK_ID = 2,
    SNOW_ID = 3,
    ICE_ID = 4;