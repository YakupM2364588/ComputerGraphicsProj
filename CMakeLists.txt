cmake_minimum_required(VERSION 3.24)
project(opengl_project)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)

# Create GLAD library and include its headers
add_library(GLAD "Source Files/glad.c"
        "Header Files/button.h"
        "Source Files/button.cpp"
)
target_include_directories(GLAD PRIVATE
        "${CMAKE_SOURCE_DIR}/includes"  # Path to "includes/glad"
)
add_executable(opengl_project
        "Source Files/main.cpp"
        "Source Files/shaderClass.cpp"
        "Source Files/EBO.cpp"
        "Source Files/VBO.cpp"
        "Source Files/VAO.cpp"
        "Source Files/bezierCurve.cpp"
        "Source Files/bezierCurvesPath.cpp"
        "Source Files/camera.cpp"
        "Source Files/light.cpp"
        "Source Files/mesh.cpp"
        "Source Files/model.cpp"
        "Source Files/railway.cpp"
        "Source Files/texture.cpp"
        "Source Files/train.cpp"
        "Source Files/tiny_obj_loader.cpp"
)

target_include_directories(opengl_project PRIVATE
        "${CMAKE_SOURCE_DIR}/includes"
        "${CMAKE_SOURCE_DIR}/Header Files"
        ${GLFW3_INCLUDE_DIRS}
        /usr/include
)

target_link_libraries(opengl_project
        glfw
        ${OPENGL_LIBRARIES}
        GLAD  # Link GLAD library
)
target_compile_definitions(opengl_project PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resource Files/")