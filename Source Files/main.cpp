#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "shaderClass.h"
#include "camera.h"
#include "model.h"
#include "bezierCurve.h"
#include "bezierCurvesPath.h"
#include "light.h"
#include "railway.h"
#include "train.h"

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void renderButton();
bool isPointInButton(double x, double y);
bool trainStopped = false;
//---------------------------------------------- Variables ----------------------------------------------
// Window dimensions
bool buttonVisible = true;

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// Train speed control
float normalSpeed = 5.0f;

// Button properties
struct Button {
    float x = 50.0f;      // Button position from left
    float y = 50.0f;      // Button position from bottom
    float width = 150.0f;
    float height = 40.0f;
    std::string text = "CONTINUE";
} speedButton;

// Control points for the bezier curves
std::vector<std::vector<glm::vec3>> curvesControlPoints = {
    {
        glm::vec3(-0.8f, -0.5f, -0.5f),
        glm::vec3(-0.3f, 0.8f, 0.2f),
        glm::vec3(0.3f, -0.8f, -0.2f),
        glm::vec3(0.8f, 0.5f, 0.5f)
    },
    {
        glm::vec3(0.8f, 0.5f, 0.5f),
        glm::vec3(1.3f, -0.2f, 0.8f),
        glm::vec3(0.2f, -0.6f, 0.3f),
        glm::vec3(-0.5f, 0.3f, -0.3f)
    },
    {
        glm::vec3(-0.5f, 0.3f, -0.3f),
        glm::vec3(-1.2f, 0.7f, -0.8f),
        glm::vec3(-0.7f, -0.2f, -0.5f),
        glm::vec3(-0.8f, -0.5f, -0.5f)
    }
};

// Global variables
Railway* g_railway = nullptr;
Train* g_train = nullptr;
glm::mat4 g_projectionMatrix;
int g_windowWidth = 800;
int g_windowHeight = 600;
std::vector<Light> g_lights;
GLFWwindow* g_window;

// Camera
Camera g_camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// GUI rendering variables
GLuint buttonVAO, buttonVBO;
Shader* buttonShader = nullptr;
//-------------------------------------------------------------------------------------------------------

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //---------------------------------------- Create render Window ----------------------------------------
    g_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics & Visual Computing Project", NULL, NULL);
    glfwMakeContextCurrent(g_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    gladLoadGL();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);

    // Set callbacks
    glfwSetKeyCallback(g_window, key_callback);

    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    //-------------------------------------------------------------------------------------------------------

    //--------------------------------------------- Framebuffer ---------------------------------------------
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    GLuint texColorBuffer;
    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //-------------------------------------------------------------------------------------------------------

    //--------------------------------------------- Quad VAO, VBO -------------------------------------------
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,

        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    GLuint quadVAO, quadVBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glBindVertexArray(0);

    // Setup button rendering
    float buttonVertices[] = {
        0.0f, 1.0f,  // Top left
        0.0f, 0.0f,  // Bottom left
        1.0f, 0.0f,  // Bottom right

        0.0f, 1.0f,  // Top left
        1.0f, 0.0f,  // Bottom right
        1.0f, 1.0f   // Top right
    };

    glGenVertexArrays(1, &buttonVAO);
    glGenBuffers(1, &buttonVBO);
    glBindVertexArray(buttonVAO);
    glBindBuffer(GL_ARRAY_BUFFER, buttonVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buttonVertices), buttonVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glBindVertexArray(0);
    //-------------------------------------------------------------------------------------------------------

    //----------------------------------------- Convolution shader ------------------------------------------
    float gaussian[9] = {
        1.0 / 16, 2.0 / 16, 1.0 / 16,
        2.0 / 16, 4.0 / 16, 2.0 / 16,
        1.0 / 16, 2.0 / 16, 1.0 / 16
    };

    Shader convolutionShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/convolution.frag");
    convolutionShader.Activate();

    for (int i = 0; i < 9; i++)
        glUniform1f(glGetUniformLocation(convolutionShader.ID, ("kernel[" + std::to_string(i) + "]").c_str()), gaussian[i]);

    glUniform1f(glGetUniformLocation(convolutionShader.ID, "offset"), 1.0f / SCR_WIDTH);
    //-------------------------------------------------------------------------------------------------------

    //----------------------------------------- Load border texture -----------------------------------------
    unsigned int borderTex;
    glGenTextures(1, &borderTex);
    glBindTexture(GL_TEXTURE_2D, borderTex);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(RESOURCE_PATH "border.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load border.jpg" << std::endl;
    }
    stbi_image_free(data);
    Shader chromakeyingShader(RESOURCE_PATH"shaders/convolution.vert", RESOURCE_PATH"shaders/chromakeying.frag");
    //-------------------------------------------------------------------------------------------------------

    // Initialize objects
    g_railway = new Railway(RESOURCE_PATH"models/rail.obj", RESOURCE_PATH"models");
    g_train = new Train(RESOURCE_PATH"models/train.obj", RESOURCE_PATH"models");

    g_lights = {
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
              {20.0f, 30.0f, 20.0f}, {1.0f, 0.0f, 0.0f}),
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
              {0.0f, 50.0f, 0.0f}, {1.0f, 1.0f, 0.0f}),
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
              {-20.0f, 30.0f, -20.0f}, {0.6f, 0.8f, 1.0f})
    };

    Shader mainShader(RESOURCE_PATH"shaders/area.vert", RESOURCE_PATH"shaders/area.frag");
    Shader lightShader(RESOURCE_PATH"shaders/sun.vert", RESOURCE_PATH"shaders/sun.frag");

    // Create button shader (simple colored rectangle)
    buttonShader = new Shader(RESOURCE_PATH"shaders/button.vert", RESOURCE_PATH"shaders/button.frag");

    //--------------------------------------------- Render loop ---------------------------------------------
    while (!glfwWindowShouldClose(g_window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        g_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = g_camera.GetViewMatrix();

        // Process input
        if (!buttonVisible) {
            g_camera.processInput(g_window, deltaTime);
        }


        // Update train with current speed
        g_train->Update(deltaTime, g_railway->GetPath());

        // STEP 1: Render scene to framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Main shader rendering
        mainShader.Activate();
        mainShader.setMat4("projection", g_projectionMatrix);
        mainShader.setMat4("view", g_camera.GetViewMatrix());
        mainShader.setVec3("viewPos", g_camera.getPosition());
        mainShader.setFloat("shininess", 32.0f);

        // Configure lights
        for (size_t i = 0; i < g_lights.size(); ++i) {
            g_lights[i].ConfigureShader(mainShader, static_cast<int>(i));
        }

        // Draw objects
        g_railway->Draw(mainShader);
        g_train->Draw(mainShader);

        // Light sources
        lightShader.Activate();
        lightShader.setMat4("projection", g_projectionMatrix);
        lightShader.setMat4("view", g_camera.GetViewMatrix());
        for (auto& light : g_lights) {
            lightShader.setVec3("lightColor", light.color);
            light.Draw(lightShader);
        }

        // STEP 2: Post-processing - render to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Apply convolution
        convolutionShader.Activate();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glUniform1i(glGetUniformLocation(convolutionShader.ID, "screenTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Apply chromakeying
        chromakeyingShader.Activate();
        glBindVertexArray(quadVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, borderTex);
        glUniform1i(glGetUniformLocation(chromakeyingShader.ID, "overlayTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render button if visible
            renderButton();

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &buttonVAO);
    glDeleteBuffers(1, &buttonVBO);
    delete buttonShader;
    delete g_railway;
    delete g_train;
    glfwDestroyWindow(g_window);
    glfwTerminate();
    return 0;
}
void renderButton() {
    if (!buttonVisible) return; // Don't render if invisible

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    buttonShader->Activate();

    float normalizedX = (speedButton.x * 2.0f) / g_windowWidth - 1.0f;
    float normalizedY = (speedButton.y * 2.0f) / g_windowHeight - 1.0f;
    float normalizedWidth = (speedButton.width * 2.0f) / g_windowWidth;
    float normalizedHeight = (speedButton.height * 2.0f) / g_windowHeight;

    glm::mat4 buttonTransform = glm::mat4(1.0f);
    buttonTransform = glm::translate(buttonTransform, glm::vec3(normalizedX, normalizedY, 0.0f));
    buttonTransform = glm::scale(buttonTransform, glm::vec3(normalizedWidth, normalizedHeight, 1.0f));

    buttonShader->setMat4("transform", buttonTransform);

    if (trainStopped) {
        buttonShader->setVec4("buttonColor", glm::vec4(0.8f, 0.2f, 0.2f, 0.8f)); // Red
    } else {
        buttonShader->setVec4("buttonColor", glm::vec4(0.2f, 0.8f, 0.2f, 0.8f)); // Green
    }

    glBindVertexArray(buttonVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisable(GL_BLEND);
}


bool isPointInButton(double x, double y) {
    // Convert screen coordinates to button coordinates
    return (x >= speedButton.x && x <= speedButton.x + speedButton.width &&
            y >= speedButton.y && y <= speedButton.y + speedButton.height);
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Convert to screen coordinates (flip Y)
        ypos = g_windowHeight - ypos;

        // Check if button was clicked
        if ( isPointInButton(xpos, ypos)) {
            if (trainStopped) {
                // Resume train
                trainStopped = false;
                g_train->speed = normalSpeed;
                speedButton.text = "STOP TRAIN";
            } else {
                // Stop train
                trainStopped = true;
                g_train->speed = 0.0f;
                speedButton.text = "CONTINUE";
            }
        }
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        buttonVisible = !buttonVisible;

        if (buttonVisible) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{    if (buttonVisible) return;  // Blokkeer camera rotatie

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    g_camera.ProcessMouseMovement(xoffset, yoffset);
}
