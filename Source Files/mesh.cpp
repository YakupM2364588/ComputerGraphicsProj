#include "mesh.h"
#include <iostream>

Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<unsigned int>& indices,
           const std::vector<Texture>& textures)
    : m_vertices(vertices), m_indices(indices), m_textures(textures),
      m_vertexBufferObject(nullptr,0), m_elementBufferObject(nullptr,0)
{
    if (vertices.empty() || indices.empty()) {
        std::cerr << "Empty vertex or index buffer!" << std::endl;
        return;
    }

    m_vertexArrayObject.Bind();

    m_vertexBufferObject = VBO(m_vertices.data(), m_vertices.size() * sizeof(Vertex));
    m_elementBufferObject = EBO(m_indices.data(), m_indices.size() * sizeof(unsigned int));

    m_vertexArrayObject.LinkVBO(m_vertexBufferObject, 0, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, position));
    m_vertexArrayObject.LinkVBO(m_vertexBufferObject, 1, 2, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
    m_vertexArrayObject.LinkVBO(m_vertexBufferObject, 2, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    m_vertexArrayObject.LinkVBO(m_vertexBufferObject, 3, 3, GL_FLOAT, sizeof(Vertex), (void*)offsetof(Vertex, color));

    m_vertexArrayObject.Unbind();
    m_vertexBufferObject.Unbind();
    m_elementBufferObject.Unbind();
}

void Mesh::Draw(const Shader& shader) const
{
    if (m_indices.empty()) {
        std::cerr << "Empty mesh";
        return;
    }

    for (auto & m_texture : m_textures) {
        if (m_texture.m_type == "texture_diffuse")
        {
            m_texture.BindToShader(shader, "diffuse_texture", 0);
            break;
        }
    }

    m_vertexArrayObject.Bind();
    //Start van begin element buffer object
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    m_vertexArrayObject.Unbind();

    glActiveTexture(GL_TEXTURE0);
}