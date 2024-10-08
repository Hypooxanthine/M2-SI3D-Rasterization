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

void Cube::load(const std::string& filePath)
{
    m_Mesh = read_indexed_mesh("data/CubeWorld/Pixel Blocks/OBJ/Grass.obj");
    m_VAO = m_Mesh.create_buffers(true, true, false, false);
}

void Cube::bind() const
{
    glBindVertexArray(m_VAO);
}