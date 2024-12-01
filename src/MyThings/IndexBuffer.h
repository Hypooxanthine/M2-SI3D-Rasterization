#pragma once

#include "Buffer.h"

template <GLenum USAGE>
using IndexBuffer = Buffer<GL_ELEMENT_ARRAY_BUFFER, USAGE>;

using StaticIndexBuffer = IndexBuffer<GL_STATIC_DRAW>;
using DynamicIndexBuffer = IndexBuffer<GL_DYNAMIC_DRAW>;