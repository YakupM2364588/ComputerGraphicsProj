#include "model.h"
#include "modelLoader.h"

//Constructor voor model met alleen meshes
Model::Model(const std::vector<Mesh>& meshes)
    : m_meshes(meshes)
{
}

//Constructor voor model van file
Model::Model(const std::string& filepath, const std::string& mtlBasePath)
{
    *this = ModelLoader::LoadOBJ(filepath, mtlBasePath);
}

//Draw alle meshes
void Model::Draw(const Shader& shader)
{
    shader.setMat4("model", GetModelMatrix());
    for (auto& mesh : m_meshes)
    {
        mesh.Draw(shader);
    }
}

void Model::AddMesh(const Mesh& mesh)
{
    m_meshes.push_back(mesh);
}

glm::mat4 Model::GetModelMatrix() const
{
    auto model = glm::mat4(1.0f);
    model = glm::translate(model, m_position);
    model = glm::rotate(model, m_rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::rotate(model, m_rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, m_rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, m_scale);
    return model;
}