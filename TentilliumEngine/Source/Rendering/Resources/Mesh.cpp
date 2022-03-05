#include "Mesh.h"
#include <glew.h>	// GL extension wrangler
#include <glfw3.h>	// GL framework



void setupAttrib(int size, unsigned int stride, unsigned int& no, unsigned int& offset)
{
	glVertexAttribPointer(no, size / 4, GL_FLOAT, GL_FALSE, stride, (void*)offset);
	glEnableVertexAttribArray(no);
	offset += size;
	no++;
}

Mesh::Mesh(void* aiMesh) { }

Mesh::Mesh(const float* vertexdata, size_t vertexsize, const unsigned int* indexdata, size_t indexsize, callback_setupVertex vertex)
	: vertexSetup(vertex)
{
	// generate buffers

	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	// bind buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	
	// set buffer data
	glBufferData(GL_ARRAY_BUFFER, vertexsize, vertexdata, GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexsize, indexdata, GL_STATIC_DRAW);

	// unbind buffers for safety
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}

Mesh::Mesh(std::vector<float>& vertexdata, std::vector<unsigned int>& indexdata, callback_setupVertex vertex)
	: vertexSetup(vertex)
{
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &IBO);

	// bind buffers
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	// set buffer data
	glBufferData(GL_ARRAY_BUFFER, vertexdata.size() * 4, &(vertexdata[0]), GL_STATIC_DRAW);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexdata.size() * 4, &(indexdata[0]), GL_STATIC_DRAW);

	// unbind buffers for safety
	glBindBuffer(GL_ARRAY_BUFFER, NULL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, NULL);
}
