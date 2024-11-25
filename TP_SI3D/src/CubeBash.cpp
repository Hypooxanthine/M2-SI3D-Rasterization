#include "CubeBash.h"

CubeBash::CubeBash(const std::string& filePath)
{
    load(filePath);
}

CubeBash::CubeBash()
{

}

CubeBash::~CubeBash()
{

}

bool CubeBash::load(const std::string& filePath)
{
    return m_Cube.load(filePath);
}

void CubeBash::reserveInstances(std::size_t nb)
{
    m_Transforms.reserve(nb);
}

void CubeBash::pushInstance(const Transform& modelMatrix)
{
    m_Transforms.push_back(modelMatrix);
}

void CubeBash::setupUniformBuffer(const Shader& shader)
{
    GLuint block = glGetUniformBlockIndex(shader.getRenderId(), "instanceData");
    ASSERT(block != GL_INVALID_INDEX, "Uniform block 'instanceData' not found in shader");

    glUniformBlockBinding(shader.getRenderId(), block, 0);

    glGenBuffers(1, &m_InstancesData);
    glBindBuffer(GL_UNIFORM_BUFFER, m_InstancesData);
    glBufferData(GL_UNIFORM_BUFFER, m_Transforms.size() * sizeof(Transform), m_Transforms.data(), GL_STATIC_DRAW);

    glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_InstancesData);

    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}


void CubeBash::draw() const
{
    m_Cube.bind();

    glDrawElementsInstanced(
        GL_TRIANGLES,
        m_Cube.getMesh().index_count(),
        GL_UNSIGNED_INT,
        nullptr,
        m_Transforms.size()
    );
}