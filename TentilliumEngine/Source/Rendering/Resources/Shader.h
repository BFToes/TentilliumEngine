#pragma once
#include "Resource.h"
#include <string>


enum ShaderType : int {
	FRAG = 0x8b30,
	GEOM = 0x8dd9,
	VERT = 0x8b31,
	COMP = 0x91b9,
};

template<ShaderType type>
class Shader : public GL<Shader<type>> {
	friend class GL<Shader<type>>;
public:
	__declspec(property(get=getHandle)) unsigned int handle;

	Shader(std::string filepath);

private:
	static void destroy(unsigned int handle);
};

using FragmentShader = Shader<FRAG>;
using VertexShader = Shader<VERT>;
using ComputeShader = Shader<COMP>;
using GeometryShader = Shader<GEOM>;