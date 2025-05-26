#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
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

GLFWwindow* g_window;
Camera g_camera(glm::vec3(0.0f, 15.0f, 30.0f), glm::vec3(0.0f, 1.0f, 0.0f), -90.0f, -40.0f);
Railway* g_railway = nullptr;
Train* g_train = nullptr;
glm::mat4 g_projectionMatrix;
int g_windowWidth = 800;
int g_windowHeight = 600;
std::vector<Light> g_lights;

GLuint fbo, rbo, texColorBuffer;
GLuint pingpongFBO[2], pingpongBuffer[2];
GLuint quadVAO, quadVBO;
unsigned int borderTex;

Shader* convolutionShader;
Shader* chromakeyingShader;
Shader* bloomShader;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);

void checkFBOStatus() {
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Framebuffer error: " << status << std::endl;
    }
}

void initializeFramebuffers() {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);

    glGenTextures(1, &texColorBuffer);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_windowWidth, g_windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);

    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, g_windowWidth, g_windowHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    checkFBOStatus();

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongBuffer);
    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, g_windowWidth, g_windowHeight, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongBuffer[i], 0);
        checkFBOStatus();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initializePostProcessing() {
    float quadVertices[] = {
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    convolutionShader = new Shader("/home/p0l/CLionProjects/OpenGLProj/resources/shaders/convolution.vert", "/home/p0l/CLionProjects/OpenGLProj/resources/shaders/convolution.frag");
    chromakeyingShader = new Shader("/home/p0l/CLionProjects/OpenGLProj/resources/shaders/chromakeying.vert", "/home/p0l/CLionProjects/OpenGLProj/resources/shaders/chromakeying.frag");
    bloomShader = new Shader("/home/p0l/CLionProjects/OpenGLProj/resources/shaders/bloom.vert", "/home/p0l/CLionProjects/OpenGLProj/resources/shaders/bloom.frag");

    glGenTextures(1, &borderTex);
    glBindTexture(GL_TEXTURE_2D, borderTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load("/home/p0l/CLionProjects/OpenGLProj/resources/textures/border.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        GLenum format = (nrChannels == 3) ? GL_RGB : GL_RGBA;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cerr << "Failed to load border texture" << std::endl;
    }
    stbi_image_free(data);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    g_window = glfwCreateWindow(g_windowWidth, g_windowHeight, "Integrated Scene", NULL, NULL);
    glfwMakeContextCurrent(g_window);
    gladLoadGL();
    glViewport(0, 0, g_windowWidth, g_windowHeight);
    glEnable(GL_DEPTH_TEST);

    initializeFramebuffers();
    initializePostProcessing();

    g_railway = new Railway("/home/p0l/CLionProjects/OpenGLProj/resources/models/rail.obj", "resources/models");
    g_train = new Train("/home/p0l/CLionProjects/OpenGLProj/resources/models/train.obj", "resources/models");

    g_lights = {
        Light("/home/p0l/CLionProjects/OpenGLProj/resources/models/sun.obj", "resources/models",
              {20.0f, 30.0f, 20.0f}, {1.0f, 0.0f, 0.0f}),
        Light("/home/p0l/CLionProjects/OpenGLProj/resources/models/sun.obj", "resources/models",
              {0.0f, 50.0f, 0.0f}, {1.0f, 1.0f, 0.0f}),
        Light("/home/p0l/CLionProjects/OpenGLProj/resources/models/sun.obj", "resources/models",
              {-20.0f, 30.0f, -20.0f}, {0.6f, 0.8f, 1.0f})
    };

    Shader mainShader("/home/p0l/CLionProjects/OpenGLProj/resources/shaders/test.vert", "/home/p0l/CLionProjects/OpenGLProj/resources/shaders/test.frag");
    Shader lightShader("/home/p0l/CLionProjects/OpenGLProj/resources/shaders/sun.vert", "/home/p0l/CLionProjects/OpenGLProj/resources/shaders/sun.frag");

    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);
    glfwSetInputMode(g_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    float lastFrame = 0.0f;
    while (!glfwWindowShouldClose(g_window)) {
        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        g_camera.processInput(g_window, deltaTime);
        g_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)g_windowWidth/g_windowHeight, 0.1f, 1000.0f);

        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClearColor(0.1f, 0.1f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mainShader.Activate();
        mainShader.setMat4("projection", g_projectionMatrix);
        mainShader.setMat4("view", g_camera.GetViewMatrix());
        mainShader.setVec3("viewPos", g_camera.getPosition());

        mainShader.setInt("numLights", g_lights.size());
        for(int i = 0; i < 3; i++) {
            std::string prefix = "lights[" + std::to_string(i) + "]";
            mainShader.setVec3(prefix + ".position", g_lights[i].position);
            mainShader.setVec3(prefix + ".ambient", 0.1f, 0.1f, 0.1f);
            mainShader.setVec3(prefix + ".diffuse", 0.8f, 0.8f, 0.8f);
            mainShader.setVec3(prefix + ".specular", 1.0f, 1.0f, 1.0f);
            mainShader.setFloat(prefix + ".constant", 1.0f);
            mainShader.setFloat(prefix + ".linear", 0.09f);
            mainShader.setFloat(prefix + ".quadratic", 0.032f);
        }

        g_railway->Draw(mainShader);
        g_train->Update(currentFrame, g_railway->GetPath());
        g_train->Draw(mainShader);

        lightShader.Activate();
        lightShader.setMat4("projection", g_projectionMatrix);
        lightShader.setMat4("view", g_camera.GetViewMatrix());
        for (auto& light : g_lights) {
            light.Draw(lightShader);
            lightShader.setVec3("lightColor", light.color);
        }


        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        convolutionShader->Activate();
        glBindVertexArray(quadVAO);
        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            convolutionShader->setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? texColorBuffer : pingpongBuffer[!horizontal]);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            horizontal = !horizontal;
            if (first_iteration) first_iteration = false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        bloomShader->Activate();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongBuffer[0]);
        bloomShader->setInt("scene", 0);
        bloomShader->setInt("bloomBlur", 1);
        glBindVertexArray(quadVAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        chromakeyingShader->Activate();
        chromakeyingShader->setVec3("keyColor", 0.0f, 1.0f, 0.0f); // Green
        chromakeyingShader->setFloat("threshold", 0.4f);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, borderTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisable(GL_BLEND);

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    // Cleanup
    delete g_railway;
    delete g_train;
    glfwTerminate();
    return 0;
}

// Callback implementations
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Raycasting
        glm::vec4 viewport(0.0f, 0.0f, g_windowWidth, g_windowHeight);
        glm::mat4 view = g_camera.GetViewMatrix();

        glm::vec3 rayOrigin = g_camera.getPosition();
        glm::vec3 worldNear = glm::unProject(
            glm::vec3(xpos, g_windowHeight - ypos, 0.0f),
            view, g_projectionMatrix, viewport
        );
        glm::vec3 rayDirection = glm::normalize(worldNear - rayOrigin);

        // Find closest rail
        float pickRadius = 5.0f;
        float closestDistance = std::numeric_limits<float>::max();
        int pickedRailIndex = -1;

        const auto& railPositions = g_railway->GetRailPositions();
        const auto& railDistances = g_railway->GetRailDistances();

        for (size_t i = 0; i < railPositions.size(); ++i) {
            glm::vec3 railPos = railPositions[i];
            glm::vec3 oc = railPos - rayOrigin;
            float t = glm::dot(oc, rayDirection);
            glm::vec3 pointOnRay = rayOrigin + rayDirection * t;
            float distance = glm::distance(railPos, pointOnRay);

            if (t > 0 && distance < pickRadius && distance < closestDistance) {
                closestDistance = distance;
                pickedRailIndex = static_cast<int>(i);
            }
        }

        // Update train position
        if (pickedRailIndex != -1 && pickedRailIndex < static_cast<int>(railDistances.size())) {
            float targetDistance = railDistances[pickedRailIndex];
            float currentTime = glfwGetTime();
            float speed = 5.0f;
            g_train->SetTimeOffset(currentTime - (targetDistance / speed));
        }
    }
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static bool firstMouse = true;
    static float lastX = g_windowWidth / 2.0f;
    static float lastY = g_windowHeight / 2.0f;

    if (firstMouse) {
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

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
    // Update projection matrix
    g_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)width/height, 0.1f, 1000.0f);
}
