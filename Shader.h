#pragma once
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>

class Shader
{
public:
	Shader();
	void CreateFromString(const char* vertexCode, const char* fragmentCode);
	void CreateFromFile(const char* vertexFile, const char* fragmentFile);
	void CreateFromFile(const char* ComputeFile); //for compute shader
	void CreateFromFile(const char* vertexFile, const char* geometryFile, const char* fragmentFile);
	void Validate();

	std::string ReadFile(const char* fileLocation);

	void UseShader();
	void ClearShader();

	void LinkShader();

	GLuint GetID() { return shaderID; }

	~Shader();
private:
	GLuint shaderID;

	void CompileShader(const char* vertexCode, const char* fragmentCode);
	void CompileShader(const char* ComputeCode);
	void CompileShader(const char* vertexCode, const char * geometryCode, const char* fragmentCode);
	void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType);

};

