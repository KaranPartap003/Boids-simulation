#include "Shader.h"

Shader::Shader()
{

}

void Shader::CreateFromString(const char* vertexCode, const char* fragmentCode)
{
    CompileShader(vertexCode, fragmentCode);
}

void Shader::CreateFromFile(const char* vertexFile, const char* fragmentFile)
{
	std::string vertexString = ReadFile(vertexFile);
	std::string fragmentString = ReadFile(fragmentFile);
	CompileShader(vertexString.c_str(), fragmentString.c_str());
}

void Shader::CreateFromFile(const char* ComputeFile)
{
	std::string ComputeString = ReadFile(ComputeFile);
	CompileShader(ComputeString.c_str());
}

void Shader::CreateFromFile(const char* vertexFile, const char* geometryFile, const char* fragmentFile)
{
	std::string vertexString = ReadFile(vertexFile);
	std::string fragmentString = ReadFile(fragmentFile);
	std::string geometryString = ReadFile(geometryFile);
	CompileShader(vertexString.c_str(), geometryString.c_str(), fragmentString.c_str());
}

std::string Shader::ReadFile(const char* fileLocation)
{
	std::string content;
	std::ifstream filestream(fileLocation, std::ios::in);

	if (!filestream.is_open()) {
		printf("failed to read from %s location", fileLocation);
		return "";
	}
	std::string line;
	while (!filestream.eof())
	{
		std::getline(filestream, line);
		content.append(line + "\n");
	}
	filestream.close();
	return content;
}

void Shader::UseShader()
{
	glUseProgram(shaderID);
}

void Shader::ClearShader()
{
	if (shaderID != 0)
	{
		glDeleteShader(shaderID);
		shaderID = 0;
	}
}

void Shader::LinkShader()
{
	glLinkProgram(shaderID);

	//----------------error checking----------------//
	GLint linkStatus;
	glGetProgramiv(shaderID, GL_LINK_STATUS, &linkStatus);

	if (linkStatus == GL_FALSE) {
		GLint logLength;
		glGetProgramiv(shaderID, GL_INFO_LOG_LENGTH, &logLength);

		char* logInfo = new char[logLength + 1];
		glGetProgramInfoLog(shaderID, logLength, nullptr, logInfo);

		std::cerr << "Shader program linking error:\n" << logInfo << std::endl;

		delete[] logInfo;
		// Handle error, cleanup, etc.
	}
	//----------------error checking----------------//
}

Shader::~Shader()
{
	Shader::ClearShader();
}


void Shader::CompileShader(const char* vertexCode, const char* fragmentCode)
{
	shaderID = glCreateProgram();
	//----------------error checking----------------//
	if (!shaderID)
	{
		printf("Failed to create shader\n");
		return;
	}
	//----------------error checking----------------//
	AddShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	AddShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

}

void Shader::CompileShader(const char* ComputeCode)
{
	shaderID = glCreateProgram();
	//----------------error checking----------------//
	if (!shaderID)
	{
		printf("Failed to create shader\n");
		return;
	}
	//----------------error checking----------------//
	AddShader(shaderID, ComputeCode, GL_COMPUTE_SHADER);
}


void Shader::CompileShader(const char* vertexCode, const char* geometryCode, const char* fragmentCode)
{
	shaderID = glCreateProgram();
	//----------------error checking----------------//
	if (!shaderID)
	{
		printf("Error creating shader program!\n");
		return;
	}
	//----------------error checking----------------//
	AddShader(shaderID, vertexCode, GL_VERTEX_SHADER);
	AddShader(shaderID, geometryCode, GL_GEOMETRY_SHADER);
	AddShader(shaderID, fragmentCode, GL_FRAGMENT_SHADER);

}

void Shader::AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	GLuint theShader = glCreateShader(shaderType);

	const GLchar* theCode[1];
	theCode[0] = shaderCode;

	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);

	glShaderSource(theShader, 1, theCode, codeLength);
	glCompileShader(theShader);
	
	//----------------error checking----------------//
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);
	if (!result)
	{
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}
	//----------------error checking----------------//

	glAttachShader(theProgram, theShader);
}

void Shader::Validate()
{
	GLint result;
	GLchar eLog[1024] = { 0 };
	glValidateProgram(shaderID);
	glGetProgramiv(shaderID, GL_VALIDATE_STATUS, &result);
	if (!result)
	{
		glGetProgramInfoLog(shaderID, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}
	return;
}

