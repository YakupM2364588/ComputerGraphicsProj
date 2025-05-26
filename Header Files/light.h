//
// Created by p0l on 5/26/25.
//

#ifndef LIGHT_H
#define LIGHT_H
#pragma once
#include <glm/glm.hpp>
#include "model.h"
#include "shaderClass.h"

class Light {
public:
    Light(const std::string& modelPath, const std::string& mtlPath,
          const glm::vec3& position, const glm::vec3& color,
          float constant = 1.0f, float linear = 0.007f, float quadratic = 0.0002f);

    void Draw(Shader& shader);
    void ConfigureShader(Shader& shader, int index) const;

    glm::vec3 position;
    glm::vec3 color;

private:
    Model model;
    float constant;
    float linear;
    float quadratic;
};

#endif //LIGHT_H
