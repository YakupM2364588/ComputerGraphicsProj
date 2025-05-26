#pragma once

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>


#include "enumCameraMovement.h"
#include "shaderClass.h"

class Camera
{
private:
	// camera Attributes
	glm::vec3 Position;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// euler Angles
	float Yaw;
	float Pitch;
	// camera options
	float MovementSpeed;
	float MouseSensitivity;

	void updateCameraVectors();
	void ProcessKeyboard(Camera_Movement direction, float deltaTime);

public:
	glm::vec3 getPosition();
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f);
	void processInput(GLFWwindow* window, float dt);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	glm::mat4 GetViewMatrix();
};

