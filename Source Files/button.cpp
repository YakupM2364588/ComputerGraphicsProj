#include "../Header Files/button.h"

Button::Button(float x, float y, float width, float height, const glm::vec3& colorID)
    : x(x), y(y), width(width), height(height),
      visible(true), isPressed(false), colorID(colorID),
      VAO(0), VBO(0), initialized(false),
      currentColor(0.0f, 0.0f, 1.0f, 0.0f)
{
}

Button::~Button() {
    if (initialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
    }
}

void Button::Initialize() {
    if (initialized) return;

    float buttonVertices[] = {
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,

        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    initialized = true;
}

//Render de button op de normale screen
void Button::Render(Shader& buttonShader, int windowWidth, int windowHeight) {
    if (!visible || !initialized) return;
    buttonShader.Activate();

    glm::mat4 transform = GetTransformMatrix(windowWidth, windowHeight);
    buttonShader.setMat4("transform", transform);
    buttonShader.setVec4("buttonColor", currentColor);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

//Render de button op de andere framebuffer object met de unqiue colorId
void Button::RenderForPicking(Shader& pickingShader, int windowWidth, int windowHeight) {
    if (!visible || !initialized) return;

    pickingShader.Activate();
    pickingShader.setVec3("pickingColor", colorID);

    glm::mat4 transform = GetTransformMatrix(windowWidth, windowHeight);
    pickingShader.setMat4("transform", transform);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


//Render de picking button met de unique kleur, check dan of het geklikte pixel die zelfde kleur heeft
bool Button::HandleClick(double mouseX, double mouseY, int windowWidth, int windowHeight,
                        GLuint pickingFBO, Shader& pickingShader) {
    if (!visible) return false;

    double flippedY = windowHeight - mouseY;

    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    RenderForPicking(pickingShader, windowWidth, windowHeight);

    GLfloat pixel[3];
    glReadPixels(mouseX, flippedY, 1, 1, GL_RGB, GL_FLOAT, &pixel);
    glm::vec3 clickedColor = glm::vec3(pixel[0], pixel[1], pixel[2]);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glm::distance(clickedColor, colorID) < 0.1f) {
        isPressed = !isPressed;
        return true;
    }

    return false; 
}

void Button::SetColor(const glm::vec4& color) {
    currentColor = color;
}

void Button::SetVisible(bool visible) {
    this->visible = visible;
}

glm::mat4 Button::GetTransformMatrix(int windowWidth, int windowHeight) const {
    float normalizedX = (x * 2.0f) / windowWidth - 1.0f;
    float normalizedY = (y * 2.0f) / windowHeight - 1.0f;
    float normalizedWidth = (width * 2.0f) / windowWidth;
    float normalizedHeight = (height * 2.0f) / windowHeight;

    glm::mat4 transform = glm::mat4(1.0f);
    transform = glm::translate(transform, glm::vec3(normalizedX, normalizedY, 0.0f));
    transform = glm::scale(transform, glm::vec3(normalizedWidth, normalizedHeight, 1.0f));

    return transform;
}