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

//---------------------------------------------- Variables ----------------------------------------------
// Window dimensions
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

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

// temporary cubus
float cubeVertices[] = {
    // Positions
    -0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
    -0.05f,  0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,

    -0.05f, -0.05f,  0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,
    -0.05f, -0.05f,  0.05f,

    -0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,
    -0.05f, -0.05f, -0.05f,
    -0.05f, -0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,

     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,

    -0.05f, -0.05f, -0.05f,
     0.05f, -0.05f, -0.05f,
     0.05f, -0.05f,  0.05f,
     0.05f, -0.05f,  0.05f,
    -0.05f, -0.05f,  0.05f,
    -0.05f, -0.05f, -0.05f,

    -0.05f,  0.05f, -0.05f,
     0.05f,  0.05f, -0.05f,
     0.05f,  0.05f,  0.05f,
     0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f,  0.05f,
    -0.05f,  0.05f, -0.05f
};
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
//-------------------------------------------------------------------------------------------------------

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //---------------------------------------- Create render Window ----------------------------------------
    GLFWwindow* g_window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics & Visual Computing Project", NULL, NULL);
    glfwMakeContextCurrent(g_window);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        throw std::runtime_error("Failed to initialize GLAD!");
    }

    gladLoadGL();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    //-------------------------------------------------------------------------------------------------------


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

	//----------------------------------------- Convolution shader ------------------------------------------
    float gaussian[9] = {
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
    };

    float laplacian[9] = {
         1,  1,  1,
         1, -7,  1,
         1,  1,  1
    };

    Shader blurShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/convolution.frag");
    Shader edgeShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/convolution.frag");
    // Gaussian kernel to blurShader
    blurShader.Activate();
    for (int i = 0; i < 9; i++)
        glUniform1f(glGetUniformLocation(blurShader.ID, ("kernel[" + std::to_string(i) + "]").c_str()), gaussian[i]);
    glUniform1f(glGetUniformLocation(blurShader.ID, "offset"), 1.0f / SCR_WIDTH);

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
    Shader chromakeyingShader( RESOURCE_PATH"shaders/convolution.vert",  RESOURCE_PATH"shaders/chromakeying.frag");
    //-------------------------------------------------------------------------------------------------------

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

    glfwSetFramebufferSizeCallback(g_window, framebuffer_size_callback);
    glfwSetCursorPosCallback(g_window, mouse_callback);
    glfwSetMouseButtonCallback(g_window, mouse_button_callback);

    //--------------------------------------------- Render loop ---------------------------------------------
    // Animation variables
    float speed = 0.5f;
    float currentDistance = 0.0f;
    // Inside your render loop, replace the current rendering section with:

while (!glfwWindowShouldClose(g_window))
{
    float currentFrame = static_cast<float>(glfwGetTime());
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    g_projectionMatrix = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = g_camera.GetViewMatrix();

    // Process input
    g_camera.processInput(g_window, deltaTime);
    g_train->Update(currentFrame, g_railway->GetPath());

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
    // Gaussian Blur
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFBO);
    glClear(GL_COLOR_BUFFER_BIT);

    blurShader.Activate();
    glBindVertexArray(quadVAO);
    glDisable(GL_DEPTH_TEST);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texColorBuffer);
    //glUniform1i(glGetUniformLocation(blurShader.ID, "screenTexture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Laplacian Edge Detection
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT);

    edgeShader.Activate();
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, intermediateTex);
    //glUniform1i(glGetUniformLocation(edgeShader.ID, "screenTexture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    // Chromakey Overlay
    chromakeyingShader.Activate();
    glBindVertexArray(quadVAO);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, borderTex);
    //glUniform1i(glGetUniformLocation(chromakeyingShader.ID, "overlayTexture"), 0);
    glDrawArrays(GL_TRIANGLES, 0, 6);

    glfwSwapBuffers(g_window);
    glfwPollEvents();
}

    glfwDestroyWindow(g_window);
    delete g_railway;
    delete g_train;
    glfwTerminate();
    return 0;
    //-------------------------------------------------------------------------------------------------------
}

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
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    g_windowWidth = width;
    g_windowHeight = height;
    glViewport(0, 0, width, height);
}
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
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