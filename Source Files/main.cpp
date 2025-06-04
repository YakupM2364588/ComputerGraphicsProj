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
#include "button.h"

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

//---------------------------------------------- Variables ----------------------------------------------
// Window dimensions
unsigned int SCR_WIDTH = 1920;
unsigned int SCR_HEIGHT = 1080;

// Button and UI state
bool buttonVisible = false;
bool trainStopped = false;
Button* g_speedButton = nullptr;
bool trainPOV = false;

// Train speed control
float normalSpeed = 5.0f;

// Picking framebuffer
GLuint pickingFBO, pickingTexture;
Shader* pickingShader = nullptr;

// Global variables
Railway* g_railway = nullptr;
Train* g_train = nullptr;
glm::mat4 g_projectionMatrix;
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

// Shaders
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

    //--------------------------------------------- Main Framebuffer ---------------------------------------------
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

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // second fbo for intermediate rendering
    GLuint intermediateFBO;
    glGenFramebuffers(1, &intermediateFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);

    GLuint intermediateTex;
    glGenTextures(1, &intermediateTex);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, intermediateTex, 0);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Intermediate framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Renderbuffer for depth and stencil
    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    //--------------------------------------------- Light-only Framebuffer ---------------------------------------------
    GLuint lightOnlyFBO;
    glGenFramebuffers(1, &lightOnlyFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, lightOnlyFBO);

    GLuint lightOnlyTex;
    glGenTextures(1, &lightOnlyTex);
    glBindTexture(GL_TEXTURE_2D, lightOnlyTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, lightOnlyTex, 0);

    // Attach same depth buffer
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Light-only framebuffer is not complete!" << std::endl;

    //---------------------------------------------  Ping-Pong Blur Framebuffers ---------------------------------------------
    GLuint pingpongFBO[2];
    GLuint pingpongTex[2];

    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongTex);

    for (unsigned int i = 0; i < 2; i++) {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongTex[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongTex[i], 0);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "ERROR::FRAMEBUFFER:: Ping-pong framebuffer " << i << " is not complete!" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //-------------------------------------------------------------------------------------------------------

    //--------------------------------------------- Picking Setup -------------------------------------------
    // Picking framebuffer setup
    glGenFramebuffers(1, &pickingFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, pickingFBO);

    // Create picking texture
    glGenTextures(1, &pickingTexture);
    glBindTexture(GL_TEXTURE_2D, pickingTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pickingTexture, 0);

    // Create picking depth buffer
    GLuint pickingDepthBuffer;
    glGenRenderbuffers(1, &pickingDepthBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, pickingDepthBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, pickingDepthBuffer);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Picking framebuffer is not complete!" << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // Create picking shader
    pickingShader = new Shader(RESOURCE_PATH"shaders/picking.vert", RESOURCE_PATH"shaders/picking.frag");
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
    //-------------------------------------------------------------------------------------------------------

    //----------------------------------------- Initialize Button ----------------------------------------
    // Create button shader
    buttonShader = new Shader(RESOURCE_PATH"shaders/button.vert", RESOURCE_PATH"shaders/button.frag");

    // Create button instance
    g_speedButton = new Button(50.0f, 50.0f, 150.0f, 40.0f, "CONTINUE", glm::vec3(1.0f, 1.0f, 1.0f));
    g_speedButton->Initialize();
    //------------------------------------------------------------------------------------------------

    //----------------------------------------- Convolution shader ------------------------------------------
    float laplacian[9] = {
         1,  1,  1,
         1, -7,  1,
         1,  1,  1
    };

    Shader blurShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/blur.frag");
    Shader edgeShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/convolution.frag");

    // Gaussian kernel to blurShader
    blurShader.Activate();
    blurShader.setInt("image", 0);

    // Gaussian blur shader for lights
    Shader lightBlurShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/blur.frag");
    lightBlurShader.Activate();
    lightBlurShader.setInt("image", 0);

    // Laplacian kernel to edgeShader
    edgeShader.Activate();
    for (int i = 0; i < 9; i++)
        glUniform1f(glGetUniformLocation(edgeShader.ID, ("kernel[" + std::to_string(i) + "]").c_str()), laplacian[i]);
    glUniform1f(glGetUniformLocation(edgeShader.ID, "offset"), 1.0f / SCR_WIDTH);
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
              {-50.0f, 10.0f, 30.0f}, {1.0f, 0.0f, 0.0f}),
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
              {50.0f, 10.0f, 30.0f}, {0.0f, 1.0f, 0.0f}),
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
              {-0.0f, 10.0f, 10.0f}, {0.0f, 0.0f, 1.0f}),
        Light(RESOURCE_PATH"models/sun.obj", RESOURCE_PATH"models",
             {0.0f, 10.0f, 80.0f}, {1.0f, 0.9f, 0.0f}),
    };

    Shader mainShader(RESOURCE_PATH"shaders/area.vert", RESOURCE_PATH"shaders/area.frag");
    Shader lightShader(RESOURCE_PATH"shaders/sun.vert", RESOURCE_PATH"shaders/sun.frag");

    //--------------------------------------------- Render loop ---------------------------------------------
    while (!glfwWindowShouldClose(g_window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        g_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = g_camera.GetViewMatrix();

        // Process input
        if (!buttonVisible || trainPOV == false) {
            g_camera.processInput(g_window, deltaTime);
        }
        if (trainPOV) {
            glm::vec3 trainFront = g_train->GetFrontPosition();
            g_camera.setPosition(trainFront + glm::vec3(0.0f, 1.5f, 0.0f));
            g_camera.ProcessMouseMovement(0.0f, 0.0f);
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

        // Render ONLY lights to separate framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, lightOnlyFBO);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render only the light sources
        lightShader.Activate();
        lightShader.setMat4("projection", g_projectionMatrix);
        lightShader.setMat4("view", g_camera.GetViewMatrix());
        for (auto& light : g_lights) {
            lightShader.setVec3("lightColor", light.color);
            light.Draw(lightShader);
        }

        // Ping-pong blur the lights
        glDisable(GL_DEPTH_TEST);

        bool horizontal = true;
        bool first_iteration = true;
        unsigned int amount = 100;

        lightBlurShader.Activate();
        glBindVertexArray(quadVAO);

        for (unsigned int i = 0; i < amount; i++) {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            lightBlurShader.setBool("horizontal", horizontal);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? lightOnlyTex : pingpongTex[!horizontal]);

            glDrawArrays(GL_TRIANGLES, 0, 6);

            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }

        // STEP 2: Post-processing - render to screen
        // Gaussian Blur
        glDisable(GL_DEPTH_TEST);
        glBindVertexArray(quadVAO);
        blurShader.Activate();

        // First pass: horizontal blur to intermediate FBO
        glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
        glClear(GL_COLOR_BUFFER_BIT);

        blurShader.setBool("horizontal", true);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Second pass: vertical blur to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        blurShader.setBool("horizontal", false);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, intermediateTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Laplacian Edge Detection
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        edgeShader.Activate();
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, intermediateTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Overlay blurred lights with blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        // Pass-through shader to render the blurred lights
        lightBlurShader.Activate();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pingpongTex[!horizontal]); // Final blurred lights
        glDrawArrays(GL_TRIANGLES, 0, 6);

        glDisable(GL_BLEND);

        // Chromakey Overlay
        chromakeyingShader.Activate();
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, borderTex);
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Render button if visible
        if (g_speedButton && buttonVisible) {
            // Update button appearance based on train state
            if (trainStopped) {
                g_speedButton->SetColor(glm::vec4(0.8f, 0.2f, 0.2f, 0.8f)); // Red
                g_speedButton->text = "CONTINUE";
            }
            else {
                g_speedButton->SetColor(glm::vec4(0.2f, 0.8f, 0.2f, 0.8f)); // Green
                g_speedButton->text = "STOP TRAIN";
            }
            g_speedButton->Render(*buttonShader, SCR_WIDTH, SCR_HEIGHT);
        }

        glfwSwapBuffers(g_window);
        glfwPollEvents();
    }

    // Cleanup
    if (g_speedButton) {
        delete g_speedButton;
    }
    delete buttonShader;
    delete pickingShader;
    glDeleteFramebuffers(1, &pickingFBO);
    glDeleteTextures(1, &pickingTexture);
    glDeleteFramebuffers(1, &lightOnlyFBO);
    glDeleteTextures(1, &lightOnlyTex);
    glDeleteFramebuffers(2, pingpongFBO);
    glDeleteTextures(2, pingpongTex);
    delete g_railway;
    delete g_train;
    glfwDestroyWindow(g_window);
    glfwTerminate();
    return 0;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && g_speedButton && buttonVisible) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Handle button click
        if (g_speedButton->HandleClick(xpos, ypos, SCR_WIDTH, SCR_HEIGHT,
                                      pickingFBO, pickingTexture, *pickingShader)) {
            // Button was clicked - toggle train state
            trainStopped = !trainStopped;
            g_train->speed = trainStopped ? 0.0f : normalSpeed;
        }
    }
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
        buttonVisible = !buttonVisible;

        if (g_speedButton) {
            g_speedButton->SetVisible(buttonVisible);
        }

        if (buttonVisible) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
    }
    if (key == GLFW_KEY_F3 && action == GLFW_PRESS) {
		trainPOV = !trainPOV;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    SCR_WIDTH = width;
    SCR_HEIGHT = height;
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    if (buttonVisible) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;  // Reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    g_camera.ProcessMouseMovement(xoffset, yoffset);
}