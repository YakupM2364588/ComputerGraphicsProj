#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shaderClass.h"
#include "bezierCurve.h"
#include "bezierCurvesPath.h"
#include <EBO.h>
#include <VBO.h>
#include <VAO.h>


//---------------------------------------------- Variables ----------------------------------------------
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

std::vector<std::vector<glm::vec3>> curvesControlPoints = {
    {
        glm::vec3(-0.8f, -0.5f, 0.0f),
        glm::vec3(-0.3f, 0.8f, 0.0f),
        glm::vec3(0.3f, -0.8f, 0.0f),
        glm::vec3(0.8f, 0.5f, 0.0f)
    },
    {
        glm::vec3(0.8f, 0.5f, 0.0f),
        glm::vec3(1.3f, -0.2f, 0.0f),
        glm::vec3(0.2f, -0.6f, 0.0f),
        glm::vec3(-0.5f, 0.3f, 0.0f)
    },
    {
        glm::vec3(-0.5f, 0.3f, 0.0f),
        glm::vec3(-1.2f, 0.7f, 0.0f),
        glm::vec3(-0.7f, -0.2f, 0.0f),
        glm::vec3(-0.8f, -0.5f, 0.0f)
    }
};
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
    
    //--------------------------------------------- Render loop ---------------------------------------------
    while (!glfwWindowShouldClose(window))
    {
        // Screen background color
        glClearColor(0.07f, 0.13f, 0.17f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw the beziercurvepath
        curveShader.Activate();
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
