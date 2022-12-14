#include "bingus.h"
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

using std::string;
using std::ifstream;
using std::stringstream;

const static char* shaderPath = "../res/shaders/";

static Shader* activeShader;

u32 LoadShader(ShaderType type, const char* filePath)
{
	string s;
	string fullPath = string(shaderPath) + filePath;
	ifstream file(fullPath);

	if (file.fail())
	{
		std::cout << "Failed to make shader! : file not found: " << fullPath << "\n";
		return -1;
	}
	else
	{
		//Read into buffer
		stringstream buffer;
		buffer << file.rdbuf();
		s = buffer.str();
	}

	const char* source = s.c_str();

	u32 id = glCreateShader(type == VERTEX ? GL_VERTEX_SHADER : GL_FRAGMENT_SHADER);
	glShaderSource(id, 1, &source, nullptr);
	glCompileShader(id);

	//Log if we failed
	int success;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		char info[512];
		glGetShaderInfoLog(id, 512, nullptr, info);
		std::cout << "Failed to make shader! : error in file @" << fullPath << "\n" << info;
		return -1;
	}

	std::cout << "Successfully made shader : @" << fullPath << "\n";
	return id;
}

Shader::Shader(u32 vertShader, u32 fragShader)
{
	uniforms = 0;
	id = glCreateProgram();
	glAttachShader(id, vertShader);
	glAttachShader(id, fragShader);
	glLinkProgram(id);

	int success;
	glGetShaderiv(id, GL_LINK_STATUS, &success);

	if (success == GL_FALSE)
	{
		char info[512];
		glGetProgramInfoLog(id, 512, nullptr, info);
		std::cout << "Failed to make shader program! : linking failed: " << info;
		return;
	}

	std::cout << "Successfully made shader program\n";
}

Shader::Shader(u32 vertShader, u32 fragShader, u32 uniformMask) : Shader(vertShader, fragShader)
{
	EnableUniforms(uniformMask);
}

Shader::Shader(const char* vert_file_path, const char* frag_file_path)
	  : Shader(LoadShader(VERTEX, vert_file_path), LoadShader(FRAGMENT, frag_file_path))
{
	//Shader(LoadShader(VERTEX, vert_file_path), LoadShader(FRAGMENT, frag_file_path));
}

Shader::Shader(const char* vert_file_path, const char* frag_file_path, u32 uniformMask)
	  : Shader(LoadShader(VERTEX, vert_file_path), LoadShader(FRAGMENT, frag_file_path), uniformMask)
{

}

void SetActiveShader(Shader* shader)
{
	if (shader == nullptr)
	{
		glUseProgram(0);
		activeShader = nullptr;	
	}
	else if (shader != activeShader)
	{
		glUseProgram(shader->id);
		activeShader = shader;
	}
}

void Shader::EnableUniform(u32 uniformID, const char* name)
{
	u32 location = glGetUniformLocation(id, name);
	assert(location != -1);
	uniformLocations[uniformID] = location;
	uniforms |= uniformID;
}

void Shader::EnableUniforms(u32 uniformMask)
{
	uniforms = uniformMask;
	
	if (HasUniform(SHADER_COLOR)) EnableUniform(SHADER_COLOR, "color");
	if (HasUniform(SHADER_MAIN_TEX)) EnableUniform(SHADER_MAIN_TEX, "main_tex");
	if (HasUniform(SHADER_SPEC_POW)) EnableUniform(SHADER_SPEC_POW, "spec_pow");
}

bool Shader::HasUniform(u32 uniform)
{
	return (uniforms & uniform) == uniform;
}

void Shader::SetUniformInt(u32 uniform, int i)
{
	SetActiveShader(this);
	glUniform1i(uniformLocations[uniform], i);
}

void Shader::SetUniformFloat(u32 uniform, float f)
{
	SetActiveShader(this);
	glUniform1f(uniformLocations[uniform], f);
}

void Shader::SetUniformVec2(u32 uniform, vec2 v2)
{
	SetActiveShader(this);
	glUniform2f(uniformLocations[uniform], v2.x, v2.y);
}

void Shader::SetUniformVec3(u32 uniform, vec3 v3)
{
	SetActiveShader(this);
	glUniform3f(uniformLocations[uniform], v3.x, v3.y, v3.z);
}

void Shader::SetUniformVec4(u32 uniform, vec4 v4)
{
	SetActiveShader(this);
	glUniform4f(uniformLocations[uniform], v4.x, v4.y, v4.z, v4.w);
}