#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "shaderClass.h"
#include "bezierCurve.h"
#include "bezierCurvesPath.h"
#include "camera.h"
#include "enumCameraMovement.h"
#include <EBO.h>
#include <VBO.h>
#include <VAO.h>


void mouse_callback(GLFWwindow* window, double xpos, double ypos);

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

// Camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Computer Graphics & Visual Computing Project", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGL();
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
    glEnable(GL_DEPTH_TEST);
    //-------------------------------------------------------------------------------------------------------
    
    //------------------------------- Create beziercurve VAO, VBO and shader --------------------------------
    BezierCurvePath path(curvesControlPoints);
    Shader curveShader(RESOURCE_PATH "shaders/curve.vert", RESOURCE_PATH "shaders/curve.frag");
    VAO curveVAO;
    curveVAO.Bind();
    const auto& vertices = path.getAllCurvesVertices();
    VBO curveVBO(vertices.data(), vertices.size() * sizeof(GLfloat));
    curveVAO.LinkVBO(curveVBO, 0);
    curveVAO.Unbind();
    curveVBO.Unbind();
    //-------------------------------------------------------------------------------------------------------

    //-------------------------------- Create temp cube VAO, VBO and shader ---------------------------------
    Shader cubeShader(RESOURCE_PATH "shaders/cube.vert", RESOURCE_PATH "shaders/cube.frag");
    VAO cubeVAO;
    cubeVAO.Bind();
    VBO cubeVBO(cubeVertices, sizeof(cubeVertices));
    cubeVAO.LinkVBO(cubeVBO, 0);
    cubeVAO.Unbind();
    cubeVBO.Unbind();
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
    //-------------------------------------------------------------------------------------------------------

	//----------------------------------------- Convolution shader ------------------------------------------
    float gaussian[9] = {
    1.0 / 16, 2.0 / 16, 1.0 / 16,
    2.0 / 16, 4.0 / 16, 2.0 / 16,
    1.0 / 16, 2.0 / 16, 1.0 / 16
    };

    float laplacian[9] = {
         0,  1,  0,
         1, -4,  1,
         0,  1,  0
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
    Shader chromakeyingShader(RESOURCE_PATH "shaders/convolution.vert", RESOURCE_PATH "shaders/chromakeying.frag");
    //-------------------------------------------------------------------------------------------------------

	//-------------------------------------------- Capture mouse --------------------------------------------
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //-------------------------------------------------------------------------------------------------------

    //--------------------------------------------- Render loop ---------------------------------------------
    // Animation variables
    float speed = 0.5f;
    float currentDistance = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Screen background color
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // Camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();

		// Process input
        camera.processInput(window, deltaTime);

		// Bind the framebuffer for offscreen rendering
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// ---------------------------------- Draw the curve ----------------------------------
		// Activate shader
        curveShader.Activate();
		curveShader.setMat4("projection", projection);
		curveShader.setMat4("view", view);
		// Bind the VAO and draw the bezier curves
        curveVAO.Bind();
        // Draw each curve
        size_t offset = 0;
        for (auto curve : path.getAllCurves()) {
            glDrawArrays(GL_LINE_STRIP, offset, curve.getCurvePoints().size());
            offset += curve.getCurvePoints().size();
        }
        curveVAO.Unbind();
        // ------------------------------------------------------------------------------------

		// ---------------------------------- Draw the cube ----------------------------------
        // Update position along path
        currentDistance += speed * deltaTime;
        if (currentDistance > path.getTotalLength()) {
            currentDistance -= path.getTotalLength();
        }

        // Get current and next positions for orientation
        glm::vec3 objectPosition = path.Evaluate(currentDistance);
        glm::vec3 nextPosition = path.Evaluate(currentDistance + 0.01f);
        glm::vec3 tangent = glm::normalize(nextPosition - objectPosition);

        // Calculate rotation to align with tangent
        glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 right = glm::normalize(glm::cross(up, tangent));
        up = glm::normalize(glm::cross(tangent, right));

        // Create model matrix for the object
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, objectPosition);

        // Create rotation matrix to align with tangent
        glm::mat4 rotation;
        rotation[0] = glm::vec4(right, 0.0f);
        rotation[1] = glm::vec4(up, 0.0f);
        rotation[2] = glm::vec4(tangent, 0.0f);
        rotation[3] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);

        model = model * rotation;

		// Activate shader
		cubeShader.Activate();
        cubeShader.setMat4("model", model);
		cubeShader.setMat4("projection", projection);
		cubeShader.setMat4("view", view);
		// Bind the VAO and draw the cube
		cubeVAO.Bind();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		cubeVAO.Unbind();
		// ------------------------------------------------------------------------------------

		// ---------------------------------- Post Processing ---------------------------------
        // convolution
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        convolutionShader.Activate();
        glBindVertexArray(quadVAO);
        glDisable(GL_DEPTH_TEST);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texColorBuffer);
        glUniform1i(glGetUniformLocation(convolutionShader.ID, "screenTexture"), 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);

		// chromakeying
        chromakeyingShader.Activate();
        glBindVertexArray(quadVAO);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, borderTex);
        glUniform1i(glGetUniformLocation(chromakeyingShader.ID, "overlayTexture"), 0);

        glDrawArrays(GL_TRIANGLES, 0, 6);
        // ------------------------------------------------------------------------------------

		// Swap the buffers
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //-------------------------------------------------------------------------------------------------------
    
    //----------------------------------------------- Clean up ----------------------------------------------
    curveVAO.Delete();
    curveVBO.Delete();

    cubeVAO.Delete();
    cubeVBO.Delete();


    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
    //-------------------------------------------------------------------------------------------------------
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}