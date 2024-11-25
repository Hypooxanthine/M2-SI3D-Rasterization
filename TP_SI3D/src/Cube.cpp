#include "Cube.h"

#include <wavefront.h>

Cube::Cube(const std::string& filePath)
{
    load(filePath);
}

Cube::Cube()
{

}

Cube::~Cube()
{
    m_Mesh.release();
}

bool Cube::load(const std::string& filePath)
{
    m_Mesh = read_indexed_mesh("data/CubeWorld/Pixel Blocks/OBJ/Grass.obj");
    if (m_Mesh == Mesh::error()) return false;

    m_VAO = m_Mesh.create_buffers(true, true, false, false);
    return true;
}

void Cube::bind() const
{
    glBindVertexArray(m_VAO);
}