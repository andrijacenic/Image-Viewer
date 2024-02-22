#include "Shader.h"
#include <iostream>

void Shader::loadShader(char* vertexSource, char* fragmentSource)
{
	if (vertexSource != nullptr) {
		vertexShaderSource = vertexSource;
	}
	if (fragmentSource != nullptr) {
		fragmentShaderSource = fragmentSource;
	}
	const char* vss = vertexShaderSource.c_str();
	vertShaderID = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShaderID, 1, &vss, nullptr);
	glCompileShader(vertShaderID);

	int success;
	char infoLog[512];
	glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertShaderID, 512, nullptr, infoLog);
		std::cerr << "Failed to compile vertex shader: " << infoLog << std::endl;
	}

	const char* fss = fragmentShaderSource.c_str();
	fragShaderID = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShaderID, 1, &fss, nullptr);
	glCompileShader(fragShaderID);

	glGetShaderiv(vertShaderID, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertShaderID, 512, nullptr, infoLog);
		std::cerr << "Failed to compile vertex shader: " << infoLog << std::endl;
	}


	shaderProgramID = glCreateProgram();
	glAttachShader(shaderProgramID, vertShaderID);
	glAttachShader(shaderProgramID, fragShaderID);
	glLinkProgram(shaderProgramID);
}

void Shader::activate()
{
	glUseProgram(shaderProgramID);
}

void Shader::deactivate()
{
	glUseProgram(0);
}

void Shader::deleteShader()
{
	glDeleteProgram(shaderProgramID);
	glDeleteShader(vertShaderID);
	glDeleteShader(fragShaderID);
}
