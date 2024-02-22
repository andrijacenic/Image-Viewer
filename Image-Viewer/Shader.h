#pragma once
#include <string>
#include <glad/glad.h>
class Shader
{
public:
	Shader(){}
	~Shader(){
		deleteShader();
	}

	void loadShader(char* vertexSource = nullptr, char* fragmentSource = nullptr);

	int fragShaderID = -1;
	int vertShaderID = -1;
	int shaderProgramID = -1;
	void activate();
	void deactivate();

	void deleteShader();
private:

	std::string vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 aPos;\n"
		"void main() {\n"
		"  gl_Position = vec4(aPos, 1.0f);\n"
		"}\n"
		"\0";

	std::string fragmentShaderSource =
		"#version 330 core\n"
		"uniform sampler2D myTexture;\n"
		"varying vec2 texCoord;\n"
		"void main() {\n"
		"	vec3 color = texture2D(myTexture, texCoord).rgb;\n"
		"	gl_FragColor = vec4(color, 1.0f); \n"
		"}\0";
};

