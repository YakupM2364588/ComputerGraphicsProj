#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

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
    
	//-------------------------------------------- Capture mouse --------------------------------------------
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    //-------------------------------------------------------------------------------------------------------

    //--------------------------------------------- Render loop ---------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Screen background color
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Process input
        camera.processInput(window, deltaTime);

		// Activate shader
        curveShader.Activate();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		curveShader.setMat4("projection", projection);
        // Camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
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

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    //-------------------------------------------------------------------------------------------------------
    
    //----------------------------------------------- Clean up ----------------------------------------------
    curveVAO.Delete();
    curveVBO.Delete();


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