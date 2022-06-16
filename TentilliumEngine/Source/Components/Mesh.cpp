#include "Mesh.h"
#include <glew.h>
#include <glfw3.h>

VAO::~VAO()
{
	if (m_handle)
		glDeleteVertexArrays(1, &m_handle);
}

VAO::VAO(VAO&& other)
{
	m_handle = other.m_handle;
	other.m_handle = 0;
}

VAO& VAO::operator=(VAO&& other)
{
	if (m_handle)
		glDeleteVertexArrays(1, &m_handle);

	m_handle = other.m_handle;
	other.m_handle = 0;

	return *this;
}

void VAO::draw()
{
	glBindVertexArray(m_handle);

	if (m_size != 0)
		glDrawElements(GL_TRIANGLES, m_size, GL_UNSIGNED_INT, NULL);
	else
		glDrawArrays(GL_TRIANGLES, 0, GL_UNSIGNED_INT);
}

void VAO::genVAO()
{
	glGenVertexArrays(1, &m_handle);
}

template<>
void VAO::attach<VertAttrib::Index>(VBO<VertAttrib::Index>* buffer)
{
	if (!buffer) return;

	glBindVertexArray(m_handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer->handle);

	glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &m_size);
	m_size /= 4;
	
	glBindVertexArray(NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

template<>
void VAO::attach<VertAttrib::Position>(VBO<VertAttrib::Position>* buffer)
{
	if (!buffer) return;
	
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
	
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);

	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

template<>
void VAO::attach<VertAttrib::Normal>(VBO<VertAttrib::Normal>* buffer)
{
	if (!buffer) return;

	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
	
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(1);

	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

template<>
void VAO::attach<VertAttrib::TexCoord>(VBO<VertAttrib::TexCoord>* buffer)
{
	if (!buffer) return;

	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
	
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(2);

	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

template<>
void VAO::attach<VertAttrib::BoneID>(VBO<VertAttrib::BoneID>* buffer)
{
	if (!buffer) return;

	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
	
	glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(3);

	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

template<>
void VAO::attach<VertAttrib::BoneWeight>(VBO<VertAttrib::BoneWeight>* buffer)
{
	if (!buffer) return;

	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, buffer->handle);
	
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(4);

	glBindVertexArray(NULL);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
}

template<>
void VAO::detach<VertAttrib::Index>()
{
	m_size = 0;
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
	glBindVertexArray(NULL);
}

template<>
void VAO::detach<VertAttrib::Position>()
{
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
}

template<>
void VAO::detach<VertAttrib::TexCoord>()
{
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
}

template<>
void VAO::detach<VertAttrib::Normal>()
{
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
	
}

template<>
void VAO::detach<VertAttrib::BoneID>()
{
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glVertexAttribPointer(3, 4, GL_INT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
}

template<>
void VAO::detach<VertAttrib::BoneWeight>()
{
	glBindVertexArray(m_handle);
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, NULL);
	glBindVertexArray(NULL);
}
