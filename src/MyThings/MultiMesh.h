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
    GLuint indexCount;
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
    // Retourne l'id du mesh ajouté.
    inline size_t addMesh(const Mesh& mesh, GLuint instanceCount = 0)
    {
        /* Tracking the mesh we are going to add */

        auto& tracker = m_MeshTrackers.emplace_back();
        tracker.firstVertex = m_Vertices.size();
        tracker.vertexCount = mesh.positions().size();
        tracker.firstIndex = m_Indices.size();
        tracker.indexCount = mesh.indices().size();

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
        
        /* Index buffer / Elements buffer */

        auto oldIndexCount = m_Indices.size();
        auto newIndexCount = oldIndexCount + mesh.indices().size();
        m_Indices.resize(newIndexCount);

        for (size_t i = oldIndexCount; i < newIndexCount; ++i)
            m_Indices.at(i) = mesh.indices().at(i - oldIndexCount);

        /* A mesh was added, we need to update buffers */
        m_NeedsUpdateMesh = true;
        m_NeedsUpdateCommand = true;

        return m_MeshTrackers.size() - 1;
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

    inline size_t getMeshCount() const { return m_MeshTrackers.size(); }
    inline constexpr const VertexArray& getVao() const { return m_VAO; }
    inline constexpr const StaticVertexBuffer& getVbo() const { return m_VBO; }
    inline constexpr const StaticIndexBuffer& getEbo() const { return m_EBO; }
    
    inline size_t addCommand(size_t meshIndex, GLuint instanceCount, GLuint baseInstance)
    {
        const auto& tracker = m_MeshTrackers.at(meshIndex);
        auto& command = m_IndirectCommands.emplace_back();

        command.indexCount = tracker.indexCount;
        command.instanceCount = instanceCount;
        command.firstIndex = tracker.firstIndex;
        command.baseVertex = tracker.firstVertex;
        command.baseInstance = baseInstance;

        return m_IndirectCommands.size() - 1;
    }

    inline void setCommand(size_t commandIndex, size_t meshIndex, GLuint instanceCount, GLuint baseInstance)
    {
        const auto& tracker = m_MeshTrackers.at(meshIndex);
        auto& command = m_IndirectCommands.at(commandIndex);

        command.indexCount = tracker.indexCount;
        command.instanceCount = instanceCount;
        command.firstIndex = tracker.firstIndex;
        command.baseVertex = tracker.firstVertex;
        command.baseInstance = baseInstance;
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
    struct MeshTracker
    {
        size_t firstVertex, vertexCount;
        size_t firstIndex, indexCount;
    };

private:
    std::vector<Vertex> m_Vertices;
    std::vector<GLuint> m_Indices;
    std::vector<MeshTracker> m_MeshTrackers;
    std::vector<DrawElementsIndirectCommand> m_IndirectCommands;
    bool m_NeedsUpdateMesh = true; // Même si on n'a ajouté aucun mesh au départ, on peut toujours créer des buffers vides...
    bool m_NeedsUpdateCommand = true;

    // GLuint m_VAO = 0;

    VertexArray m_VAO;
    StaticVertexBuffer m_VBO;
    StaticIndexBuffer m_EBO;
    DynamicDrawIndirectBO m_DrawIndirectBO;
};