#include "CubeBash.h"

CubeBash::CubeBash()
{

}

CubeBash::~CubeBash()
{

}

void CubeBash::load(const std::string& filePath, GLuint program, GLuint uniformBlock, GLuint blockAttachment)
{
    m_Cube.load(filePath);

    glUniformBlockBinding(program, uniformBlock, blockAttachment);
    glBindBufferBase(GL_UNIFORM_BUFFER, uniformBlock, m_InstancesData);
}

void CubeBash::reserveInstances(std::size_t nb)
{
    m_Transforms.reserve(nb);
}

void CubeBash::pushInstance(const Transform& modelMatrix)
{
    m_Transforms.push_back(modelMatrix);
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