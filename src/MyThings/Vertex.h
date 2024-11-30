#pragma once

#include "mat.h"

struct Vertex
{
    vec3 position = { 0.f, 0.f, 0.f };
    vec2 texcoords = { 0.f, 0.f };
    vec3 normal = { 0.f, 0.f, 0.f };
};