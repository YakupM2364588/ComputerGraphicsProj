#ifndef BUTTON_H
#define BUTTON_H

#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "shaderClass.h"

class Button {
public:
    float x, y, width, height;
    bool visible;
    bool isPressed;

    glm::vec3 colorID;

    Button(float x = 50.0f, float y = 50.0f, float width = 150.0f, float height = 40.0f, const glm::vec3& colorID = glm::vec3(1.0f, 1.0f, 1.0f));

    ~Button();

    void Initialize();

    void Render(Shader& buttonShader, int windowWidth, int windowHeight);

    void RenderForPicking(Shader& pickingShader, int windowWidth, int windowHeight);

    bool HandleClick(double mouseX, double mouseY, int windowWidth, int windowHeight,
                    GLuint pickingFBO, Shader& pickingShader);

    void SetColor(const glm::vec4& color);

    void SetVisible(bool visible);

private:
    GLuint VAO, VBO;
    bool initialized;
    glm::vec4 currentColor;

    glm::mat4 GetTransformMatrix(int windowWidth, int windowHeight) const;
};

#endif //BUTTON_H
