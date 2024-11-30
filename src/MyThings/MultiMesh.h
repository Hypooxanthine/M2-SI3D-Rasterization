#pragma once

#include <vector>

#include "glcore.h"
#include "mesh.h"

#include "Vertex.h"

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
 * pour utiliser glMultiDrawElementsIndirect plus tard
 */
class MultiMesh
{
public:
    inline constexpr MultiMesh()
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
        auto newVertexCount = oldVertexCount + mesh.vertex_buffer_size();
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
        auto newIndexCount = oldIndexCount + mesh.index_count();
        m_Indices.resize(newIndexCount);

        for (size_t i = oldIndexCount; i < newIndexCount; ++i)
            m_Indices.at(i) = mesh.indices().at(i - oldIndexCount);

        command.count = newIndexCount - oldIndexCount;
        command.firstIndex = oldIndexCount;

        /* Values for instacesCount and baseInstance */
        command.instanceCount = instanceCount;
        command.baseInstance = 0;

        /* A mesh was added, we need to update buffers */
        m_NeedsUpdateMesh = true;
        m_NeedsUpdateCommand = true;
    }

    inline GLuint createBuffers()
    {
        if (m_NeedsUpdateMesh) updateBuffers();

        return m_VAO;
    }

    inline void draw(GLenum mode = GL_TRIANGLES) const
    {
        glBindVertexArray(m_VAO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBO);

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
        if (m_VAO != 0) glDeleteVertexArrays(1, &m_VAO);
        if (m_VBO != 0) glDeleteBuffers(1, &m_VBO);
        if (m_EBO != 0) glDeleteBuffers(1, &m_EBO);

        m_VAO = 0;
        m_VBO = 0;
        m_EBO = 0;

        m_NeedsUpdateMesh = true;
    }

    inline void releaseCommandsBuffer()
    {
        if (m_DrawIndirectBO != 0) glDeleteBuffers(1, &m_DrawIndirectBO);
        m_DrawIndirectBO = 0;
        m_NeedsUpdateCommand = true;
    }

    inline constexpr size_t getMeshCount() const { return m_IndirectCommands.size(); }
    inline constexpr GLuint getVao() const { return m_VAO; }
    inline constexpr GLuint getVbo() const { return m_VBO; }
    inline constexpr GLuint getEbo() const { return m_EBO; }
    inline constexpr GLuint getMeshInstanceCount(size_t meshIndex) const { return m_IndirectCommands.at(meshIndex).instanceCount; }
    inline constexpr void setMeshInstanceCount(size_t meshindex, size_t instanceCount) { m_IndirectCommands.at(meshindex).instanceCount = instanceCount; }

    inline void updateBuffers()
    {
        // Nettoyage des buffers s'ils ont déjà été créés
        releaseMeshBuffers();

        // Création des buffers
        glGenBuffers(1, &m_VBO);
        glGenBuffers(1, &m_EBO);
        glGenVertexArrays(1, &m_VAO);

        // Contenu du vertex buffer (VBO)
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(
            GL_ARRAY_BUFFER,
            m_Vertices.size() * sizeof(decltype(m_Vertices)::value_type),
            m_Vertices.data(),
            GL_STATIC_DRAW
        );

        // Contenu de l'index buffer/element buffer (IBO/EBO)
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            m_Indices.size() * sizeof(decltype(m_Indices)::value_type),
            m_Indices.data(),
            GL_STATIC_DRAW
        );

        // Paramétrage du vertex array (VAO)
        glBindVertexArray(m_VAO);
        // Positions
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(
                0, // Index de l'attribut
                sizeof(decltype(Vertex::position)), // Taille de l'élement position
                GL_FLOAT, // Type de l'élément position sur le gpu
                GL_FALSE, // Non normalisé
                sizeof(Vertex), // Nombre d'octets entre le début de deux attributs consécutifs dans le VBO
                (void*)offsetof(Vertex, position) // Position du premier élément dans le VBO
            );
        // Coordonnées de texture
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(
                1,
                sizeof(decltype(Vertex::texcoords)),
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                (void*)offsetof(Vertex, texcoords)
            );
        // Normales
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(
                2,
                sizeof(decltype(Vertex::normal)),
                GL_FLOAT,
                GL_FALSE,
                sizeof(Vertex),
                (void*)offsetof(Vertex, normal)
            );

        // C'est terminé
        m_NeedsUpdateMesh = false;

        if (m_NeedsUpdateCommand) updateCommandsBuffer();
    }

    inline void updateCommandsBuffer()
    {
        // Nettoyage (pourrait peut-être amélioré en faisant du subdata, mais ça ferait potentiellement beaucoup d'aller-retours cpu/gpu)
        releaseCommandsBuffer();

        glGenBuffers(1, &m_DrawIndirectBO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, m_DrawIndirectBO);
        glBufferData(
            GL_DRAW_INDIRECT_BUFFER,
            m_IndirectCommands.size() * sizeof(decltype(m_IndirectCommands)::value_type),
            m_IndirectCommands.data(),
            GL_DYNAMIC_DRAW // Sera souvent mis à jour
        );

        // C'est terminé
        m_NeedsUpdateCommand = true;
    }

private:
    std::vector<Vertex> m_Vertices;
    std::vector<GLuint> m_Indices;
    std::vector<DrawElementsIndirectCommand> m_IndirectCommands;
    bool m_NeedsUpdateMesh = true; // Même si on n'a ajouté aucun mesh au départ, on peut toujours créer des buffers vides...
    bool m_NeedsUpdateCommand = true;

    GLuint m_VBO = 0, m_EBO = 0, m_VAO = 0, m_DrawIndirectBO = 0;
};