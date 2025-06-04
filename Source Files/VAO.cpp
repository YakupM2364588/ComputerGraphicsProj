#include"VAO.h"

// Constructor that generates a VAO ID
VAO::VAO()
{
	glGenVertexArrays(1, &ID);
}
void VAO::LinkVBO(VBO& VBO, GLuint layout)
{
	VBO.Bind();
	glVertexAttribPointer(layout, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}
// Links a VBO to the VAO using a certain layout
void VAO::LinkVBO(VBO& VBO, GLuint layout, GLint components,
				 GLenum type, GLsizei stride, const void* offset) {
	VBO.Bind();
	glVertexAttribPointer(layout, components, type, GL_FALSE, stride, offset);
	glEnableVertexAttribArray(layout);
	VBO.Unbind();
}

// Binds the VAO
void VAO::Bind() const {
	glBindVertexArray(ID);
}

// Unbinds the VAO
void VAO::Unbind() const
{
	glBindVertexArray(0);
}
