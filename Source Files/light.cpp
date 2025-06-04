#include "light.h"

Light::Light(const std::string& modelPath, const std::string& mtlPath,
             const glm::vec3& position, const glm::vec3& color,
             float constant, float linear, float quadratic)
    : m_model(modelPath, mtlPath), position(position), color(color),
      m_constant(constant), m_linear(linear), m_quadratic(quadratic) {
    m_model.SetScale(glm::vec3(0.05f));
}

//Draw de model
void Light::Draw(Shader& shader) {
    m_model.SetPosition(position);
    m_model.Draw(shader);
}

//Cnfigure het voor de
void Light::ConfigureShader(Shader& shader, int index) const {
    std::string prefix = "lights[" + std::to_string(index) + "]";
    shader.setVec3(prefix + ".position", position);
    shader.setVec3(prefix + ".ambient", color * 0.1f);
    shader.setVec3(prefix + ".diffuse", color * 1.5f);
    shader.setVec3(prefix + ".specular", glm::vec3(0.5f));
    shader.setFloat(prefix + ".constant", m_constant);
    shader.setFloat(prefix + ".linear", m_linear);
    shader.setFloat(prefix + ".quadratic", m_quadratic);
}