#ifndef LIGHT_H
#define LIGHT_H
#include <glm/glm.hpp>
#include "model.h"
#include "shaderClass.h"
#include "glm/vec3.hpp"

class Light {
public:
    Light(const std::string& modelPath, const std::string& mtlPath,
          const glm::vec3& position, const glm::vec3& color,
          float constant = 1.0f, float linear = 0.005f, float quadratic = 0.0005f);

    void Draw(Shader& shader);
    void ConfigureShader(Shader& shader, int index) const;

    glm::vec3 position;
    glm::vec3 color;

private:
    Model m_model;
    float m_constant;
    float m_linear;
    float m_quadratic;
};

#endif //LIGHT_H
