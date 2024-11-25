#pragma once

#include "Cube.h"

#include "Shader.h"

class CubeBash
{
public:
    CubeBash(const std::string& filePath);
    CubeBash();
    ~CubeBash();

    bool load(const std::string& filePath);

    void reserveInstances(std::size_t nb);
    void pushInstance(const Transform& modelMatrix);

    void setupUniformBuffer(const Shader& shader);

    void draw() const;

private:
    Cube m_Cube;
    GLuint m_InstancesData;
    std::vector<Transform> m_Transforms;
};