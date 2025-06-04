#pragma once

#include<glad/glad.h>
#include"VBO.h"

class VAO
{
public:
	// ID reference for the Vertex Array Object
	GLuint ID;
	// Constructor that generates a VAO ID
	VAO();

	// Links a VBO to the VAO using a certain layout
	void LinkVBO(VBO& VBO, GLuint layout, GLint components,
				 GLenum type, GLsizei stride, const void* offset);

	void LinkVBO(VBO &VBO, GLuint layout);;
	// Binds the VAO
	void Bind() const;
	// Unbinds the VAO
	void Unbind() const;

};