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
	glUseProgram(shaderProgramID);

	glGenBuffers(1, &posData);
	glGenBuffers(1, &colorData);
	glGenBuffers(1, &uvData);

	glUseProgram(0);
}

void Shader::drawImageWithModification(int texID, Image* image)
{
	glUseProgram(shaderProgramID);

	GLfloat positions[]={
		image->mod.positions[0].x, image->mod.positions[0].y, 0,
		image->mod.positions[1].x, image->mod.positions[1].y, 0,
		image->mod.positions[2].x, image->mod.positions[2].y, 0,
		image->mod.positions[3].x, image->mod.positions[3].y, 0
	};
	GLfloat colors[] = {
		image->mod.colors[0].x, image->mod.colors[0].y, image->mod.colors[0].z, image->mod.colors[0].w,
		image->mod.colors[1].x, image->mod.colors[1].y, image->mod.colors[1].z, image->mod.colors[1].w,
		image->mod.colors[2].x, image->mod.colors[2].y, image->mod.colors[2].z, image->mod.colors[2].w,
		image->mod.colors[3].x, image->mod.colors[3].y, image->mod.colors[3].z, image->mod.colors[3].w
	};
	GLfloat uv[]{
		image->uv[0].x, image->uv[0].y,
		image->uv[1].x, image->uv[1].y,
		image->uv[2].x, image->uv[2].y,
		image->uv[3].x, image->uv[3].y
	};

	glBindBuffer(GL_ARRAY_BUFFER, posData);
	glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), positions, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, colorData);
	glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), colors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, uvData);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), uv, GL_STATIC_DRAW);


	GLuint attribPosition = glGetAttribLocation(shaderProgramID, "inPosition");
	glEnableVertexAttribArray(attribPosition);
	glBindBuffer(GL_ARRAY_BUFFER, posData);
	glVertexAttribPointer(attribPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint attribColor = glGetAttribLocation(shaderProgramID, "inColor");
	glEnableVertexAttribArray(attribColor);
	glBindBuffer(GL_ARRAY_BUFFER, colorData);
	glVertexAttribPointer(attribColor, 4, GL_FLOAT, GL_FALSE, 0, 0);

	GLuint attribUv = glGetAttribLocation(shaderProgramID, "inUv");
	glEnableVertexAttribArray(attribUv);
	glBindBuffer(GL_ARRAY_BUFFER, uvData);
	glVertexAttribPointer(attribUv, 2, GL_FLOAT, GL_FALSE, 0, 0);

	GLint textureLocation = glGetUniformLocation(shaderProgramID, "sampler");
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texID);
	glUniform1i(textureLocation, 0);

	U1f("contrast", image->mod.contrast);
	U1f("saturation", image->mod.saturation);
	U1f("hue", image->mod.hue / 360.0);

	glDrawArrays(GL_QUADS, 0, 4);

	glUseProgram(0);
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

	glDeleteBuffers(1, &posData);
	glDeleteBuffers(1, &colorData);
	glDeleteBuffers(1, &uvData);
}

void Shader::U1f(const char* uName, float uValue) {
	glUniform1f(glGetUniformLocation(shaderProgramID, uName), static_cast<GLfloat>(uValue));
}

void Shader::U2f(const char* uName, float uValue, float uValue2) {
	glUniform2f(glGetUniformLocation(shaderProgramID, uName), static_cast<GLfloat>(uValue), static_cast<GLfloat>(uValue2));
}

void Shader::U3f(const char* uName, float uValue, float uValue2, float uValue3) {
	glUniform3f(glGetUniformLocation(shaderProgramID, uName), static_cast<GLfloat>(uValue), static_cast<GLfloat>(uValue2), static_cast<GLfloat>(uValue3));
}

void Shader::U1i(const char* uName, int uValue) {
	glUniform1i(glGetUniformLocation(shaderProgramID, uName), uValue);
}

void Shader::U1fv(const char* uName, int uValue, float uValue2[]) {
	glUniform1fv(glGetUniformLocation(shaderProgramID, uName), uValue, uValue2);
}