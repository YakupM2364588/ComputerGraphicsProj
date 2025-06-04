#pragma once

#include<glad/glad.h>

class VBO
{
public:
	// Reference ID of the Vertex Buffer Object
	GLuint ID;
	// Constructor that generates a Vertex Buffer Object and links it to vertices
	VBO(const void* data, GLsizeiptr size);

	// Binds the VBO
	void Bind() const;
	// Unbinds the VBO
	void Unbind() const;
};