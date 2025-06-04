#include "mesh.h"

//Init de VBO, VAO, EBO
Mesh::Mesh(const std::vector<Vertex>& vertices,
           const std::vector<unsigned int>& indices,
           const std::vector<Texture>& textures)
    : m_vertices(vertices), m_indices(indices), m_textures(textures),
      m_vertexBufferObject(nullptr,0), m_elementBufferObject(nullptr,0)
{
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

//Draw de vertices, in de VBO
void Mesh::Draw(const Shader& shader) const
{
    if (m_indices.empty()) {
        return;
    }
    m_vertexArrayObject.Bind();
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices.size()), GL_UNSIGNED_INT, 0);
    m_vertexArrayObject.Unbind();
    glActiveTexture(GL_TEXTURE0);
}