#pragma once

#include <string>

#include <mesh.h>

class Cube
{
public:
    Cube(const std::string& filePath);
    Cube();
    ~Cube();

    bool load(const std::string& filePath);

    inline const Mesh& getMesh() const { return m_Mesh; }

    void bind() const;

private:
    Mesh m_Mesh;
    GLuint m_VAO;
};