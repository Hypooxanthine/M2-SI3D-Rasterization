#pragma once

#include "Cube.h"

class CubeBash
{
public:
    CubeBash(const std::string& filePath);
    CubeBash();
    ~CubeBash();

    void load(const std::string& filePath, GLuint program, GLuint uniformBlock, GLuint blockAttachment);

    void reserveInstances(std::size_t nb);
    void pushInstance(const Transform& modelMatrix);

    void draw() const;

private:
    Cube m_Cube;
    GLuint m_InstancesData;
    std::vector<Transform> m_Transforms;
};