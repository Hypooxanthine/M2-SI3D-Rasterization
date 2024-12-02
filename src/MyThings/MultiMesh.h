#pragma once

#include <vector>
#include <iostream>

#include "glcore.h"
#include "mesh.h"

#include "Vertex.h"

#include "VertexArray.h"
#include "DrawIndirectBO.h"

// Commande pour 1 mesh
struct DrawElementsIndirectCommand
{
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLint  baseVertex;
    GLuint baseInstance;
};

/**
 * Stocke plusieurs VBO et EBO concaténés et garde en mémoire leurs positions
 * pour utiliser glMultiDrawElementsIndirect plus tard (donc au final, 1 VBO et 1 EBO)
 */
class MultiMesh
{
public:
    inline MultiMesh()
    {

    }

    inline ~MultiMesh()
    {
        release();
    }

    // Ne va pas créer les buffers via Mesh::create_buffers(). Ils seront recréés par la classe MultiMesh.
    inline void addMesh(const Mesh& mesh, GLuint instanceCount = 0)
    {
        auto& command = m_IndirectCommands.emplace_back();

        /* Vertex buffer */

        auto oldVertexCount = m_Vertices.size();
        auto newVertexCount = oldVertexCount + mesh.positions().size();
        m_Vertices.resize(newVertexCount);

        if (mesh.has_position())
            for (size_t i = oldVertexCount; i < newVertexCount; ++i)
                m_Vertices.at(i).position = mesh.positions().at(i - oldVertexCount);

        if (mesh.has_texcoord())
            for (size_t i = oldVertexCount; i < newVertexCount; ++i)
                m_Vertices.at(i).texcoords = mesh.texcoords().at(i - oldVertexCount);

        if (mesh.has_normal())
            for (size_t i = oldVertexCount; i < newVertexCount; ++i)
                m_Vertices.at(i).normal = mesh.normals().at(i - oldVertexCount);
                
        command.baseVertex = oldVertexCount;
        
        /* Index buffer / Elements buffer */

        auto oldIndexCount = m_Indices.size();
        auto newIndexCount = oldIndexCount + mesh.indices().size();
        m_Indices.resize(newIndexCount);

        for (size_t i = oldIndexCount; i < newIndexCount; ++i)
            m_Indices.at(i) = mesh.indices().at(i - oldIndexCount);

        command.count = newIndexCount - oldIndexCount;
        command.firstIndex = oldIndexCount;

        /* Values for instacesCount and baseInstance */
        command.instanceCount = instanceCount;

        if (getMeshCount() == 1)
            command.baseInstance = 0;
        else
            command.baseInstance = m_IndirectCommands[getMeshCount() - 1].baseInstance + m_IndirectCommands[getMeshCount() - 1].instanceCount;

        /* A mesh was added, we need to update buffers */
        m_NeedsUpdateMesh = true;
        m_NeedsUpdateCommand = true;
    }

    inline const VertexArray& createBuffers()
    {
        if (m_NeedsUpdateMesh) updateBuffers();

        return m_VAO;
    }

    inline void draw(GLenum mode = GL_TRIANGLES) const
    {
        m_VAO.bind();
        m_EBO.bind();
        m_DrawIndirectBO.bind();

        glMultiDrawElementsIndirect(
            mode,
            GL_UNSIGNED_INT,
            nullptr,
            m_IndirectCommands.size(),
            0
        );
    }

    inline void release()
    {
        releaseMeshBuffers();
        releaseCommandsBuffer();
        m_Vertices.clear();
        m_Indices.clear();
        m_IndirectCommands.clear();
    }

    inline void releaseMeshBuffers()
    {
        m_VAO.release();
        m_VBO.release();
        m_EBO.release();

        m_NeedsUpdateMesh = true;
    }

    inline void releaseCommandsBuffer()
    {
        m_DrawIndirectBO.release();
        m_NeedsUpdateCommand = true;
    }

    inline size_t getMeshCount() const { return m_IndirectCommands.size(); }
    inline constexpr const VertexArray& getVao() const { return m_VAO; }
    inline constexpr const StaticVertexBuffer& getVbo() const { return m_VBO; }
    inline constexpr const StaticIndexBuffer& getEbo() const { return m_EBO; }
    inline GLuint getMeshInstanceCount(size_t meshIndex) const { return m_IndirectCommands.at(meshIndex).instanceCount; }
    inline void setMeshInstanceCount(size_t meshIndex, size_t instanceCount)
    {
        m_IndirectCommands.at(meshIndex).instanceCount = instanceCount;

        for (size_t i = meshIndex + 1; i < m_IndirectCommands.size(); ++i)
        {
            auto& command = m_IndirectCommands.at(i);

            if (i == 0)
                command.baseInstance = 0;
            else
                command.baseInstance = m_IndirectCommands.at(i - 1).baseInstance + m_IndirectCommands.at(i - 1).instanceCount;
        }

        m_NeedsUpdateCommand = true;
    }

    inline void updateBuffers()
    {
        // Nettoyage des buffers s'ils ont déjà été créés
        releaseMeshBuffers();

        // Contenu du vertex buffer (VBO)
        m_VBO.generateVertices(m_Vertices.data(), m_Vertices.size());

        // Contenu de l'index buffer/element buffer (IBO/EBO)
        m_EBO.generateIndices(m_Indices.data(), m_Indices.size());

        // Paramétrage du vertex array (VAO)
        VertexBufferLayout layout;
        // Positions
        layout.pushFloats(3);
        // Coordonnées de texture
        layout.pushFloats(2);
        // Normales
        layout.pushFloats(3);
        
        m_VAO.generate();
        m_VAO.addLayout(m_VBO, layout);

        // C'est terminé
        m_NeedsUpdateMesh = false;

        if (m_NeedsUpdateCommand) updateCommandsBuffer();
    }

    inline void updateCommandsBuffer()
    {
        // Nettoyage (pourrait peut-être amélioré en faisant du subdata, mais ça ferait potentiellement beaucoup d'aller-retours cpu/gpu)
        releaseCommandsBuffer();
        
        m_DrawIndirectBO.generateCommands(m_IndirectCommands.data(), m_IndirectCommands.size());

        // C'est terminé
        m_NeedsUpdateCommand = true;
    }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<GLuint> m_Indices;
    std::vector<DrawElementsIndirectCommand> m_IndirectCommands;
    bool m_NeedsUpdateMesh = true; // Même si on n'a ajouté aucun mesh au départ, on peut toujours créer des buffers vides...
    bool m_NeedsUpdateCommand = true;

    // GLuint m_VAO = 0;

    VertexArray m_VAO;
    StaticVertexBuffer m_VBO;
    StaticIndexBuffer m_EBO;
    DynamicDrawIndirectBO m_DrawIndirectBO;
};