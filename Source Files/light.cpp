#include "glm/vec3.hpp"
//
// Created by p0l on 5/26/25.
//
#include "light.h"

Light::Light(const std::string& modelPath, const std::string& mtlPath,
             const glm::vec3& position, const glm::vec3& color,
             float constant, float linear, float quadratic)
    : model(modelPath, mtlPath), position(position), color(color),
      constant(constant), linear(linear), quadratic(quadratic) {
    model.SetScale(glm::vec3(0.05f));
}

void Light::Draw(Shader& shader) {
    model.SetPosition(position);
    model.Draw(shader);
}

void Light::ConfigureShader(Shader& shader, int index) const {
    std::string prefix = "lights[" + std::to_string(index) + "]";
    shader.setVec3(prefix + ".position", position);
    shader.setVec3(prefix + ".ambient", color * 0.1f);
    shader.setVec3(prefix + ".diffuse", color * 1.5f);
    shader.setVec3(prefix + ".specular", glm::vec3(0.5f));
    shader.setFloat(prefix + ".constant", constant);
    shader.setFloat(prefix + ".linear", linear);
    shader.setFloat(prefix + ".quadratic", quadratic);
}