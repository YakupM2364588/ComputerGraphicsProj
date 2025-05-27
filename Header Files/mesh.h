//
// Created by p0l on 5/19/25.
//

#ifndef MESH_H
#define MESH_H
#include <vector>
#include "vertex.h"
#include "EBO.h"
#include "texture.h"
#include "VAO.h"
class Mesh {
    public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
    void Draw(const Shader &shader) const;

    void Init();


    VAO m_vertexArrayObject;
    EBO m_elementBufferObject;
    VBO m_vertexBufferObject;
    std::vector<Texture> m_textures;
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;

};
#endif //MESH_H
