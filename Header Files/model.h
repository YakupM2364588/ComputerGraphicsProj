#ifndef MODEL_H
#define MODEL_H

#include <vector>
#include <string>
#include "mesh.h"
#include "shaderClass.h"
#include "glm/gtc/matrix_transform.hpp"
class Model {
public:
    Model(const std::vector<Mesh>& meshes);

    Model(const std::string& filepath, const std::string& mtlBasePath = "");

    void Draw(const Shader& shader);

    void AddMesh(const Mesh& mesh);

    std::vector<Mesh>& GetMeshes() { return m_meshes; }
    const std::vector<Mesh>& GetMeshes() const { return m_meshes; }

    void SetPosition(const glm::vec3& position) { m_position = position; }
    void SetRotation(const glm::vec3& rotation) { m_rotation = rotation; }
    void SetScale(const glm::vec3& scale) { m_scale = scale; }

    glm::vec3 GetPosition() const { return m_position; }
    glm::vec3 GetRotation() const { return m_rotation; }
    glm::vec3 GetScale() const { return m_scale; }

    glm::mat4 GetModelMatrix() const;

private:
    std::vector<Mesh> m_meshes;

    glm::vec3 m_position = glm::vec3(0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f);
};

#endif // MODEL_H