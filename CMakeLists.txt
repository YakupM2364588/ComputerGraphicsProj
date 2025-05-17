cmake_minimum_required(VERSION 3.24)
project(opengl_project)

set(CMAKE_CXX_STANDARD 17)

link_directories(${CMAKE_SOURCE_DIR}/lib)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

find_package(OpenGL REQUIRED)

# find the required packages
find_package(GLM REQUIRED)
message(STATUS "GLM included at ${GLM_INCLUDE_DIR}")
find_package(GLFW3 REQUIRED)
message(STATUS "Found GLFW3 in ${GLFW3_INCLUDE_DIR}")

set(LIBS glfw3 opengl32)

add_library(GLAD "Source Files/glad.c")
set(LIBS ${LIBS} GLAD)

include_directories(${CMAKE_SOURCE_DIR}/includes)
include_directories("${CMAKE_SOURCE_DIR}/Header Files")
add_executable(opengl_project "Source Files/main.cpp" "Source Files/shaderClass.cpp" "Header Files/stb_image.h" "Source Files/EBO.cpp" "Source Files/VBO.cpp" "Source Files/VAO.cpp" "Source Files/bezierCurve.cpp" "Source Files/bezierCurvesPath.cpp" "Source Files/camera.cpp")
target_link_libraries(opengl_project ${LIBS})
target_compile_definitions(opengl_project PRIVATE RESOURCE_PATH="${CMAKE_SOURCE_DIR}/Resource Files/")
