#include <stdio.h>
#include <stdlib.h>

#include <GL/glew.h>
#include "stb_image.h"

char* readFile(const char path[]);
void compileShader(GLuint* shader,GLenum type,const char path[]);
void attachShaders(unsigned int* shaderProgram, unsigned int vertexShader,unsigned int fragmentShader);
void loadTexture(unsigned int* texture, const char path[], bool transparent);

char* readFile(const char path[])
{
	unsigned int len = 1;
	unsigned int index = 0;
	char* str = (char *)malloc(sizeof(char) * len);
	char* temp;
	char c;
	FILE* file = fopen(path, "r");
	if (!file)
		goto error;
	c = getc(file);
	while (c != EOF)
	{
		str[index] = c; index++;
		if (index == len) {
			len *= 2;
			if (NULL == (temp = (char *)realloc(str, sizeof(char) * len)))
				goto error;
			else
				str = temp;
		}
		c = getc(file);
	}
	if (NULL == (temp = (char *)realloc(str, sizeof(char) * (index + 1))))
		goto error;
	else
		str = temp;
	str[index] = '\0';
	fclose(file);

	return str;
error:
	free(str);
	free(temp);
	fclose(file);
	printf("Error reading file at path:\n%s\n", path);
	exit(-1);
}

void compileShader(GLuint* shader,GLenum type,const char path[])
{
	*shader = glCreateShader(type);
	const char* shaderSource = readFile(path);
	glShaderSource(*shader, 1, &shaderSource, NULL);
	glCompileShader(*shader);
	free((void *)shaderSource);

	int success = 0;
	char infoLog[512];
	glGetShaderiv(*shader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(*shader, 512, NULL, infoLog);
		fprintf(stderr, "Error compiling the %d shader: '%s'\n", type, infoLog);
		exit(-1);
	}
}

void attachShaders(unsigned int* shaderProgram, unsigned int vertexShader,unsigned int fragmentShader)
{
	*shaderProgram = glCreateProgram();
	glAttachShader(*shaderProgram, vertexShader);
	glAttachShader(*shaderProgram, fragmentShader);
	glLinkProgram(*shaderProgram);

	int success = 0;
	char infoLog[512];
	glGetProgramiv(*shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(*shaderProgram, 512, NULL, infoLog);
		fprintf(stderr, "Error linking to program: '%s'\n", infoLog);
		exit(-1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
}

void loadTexture(unsigned int* texture, const char path[], bool transparent)
{
	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, *texture);

	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);
	if (!data) {
		stbi_image_free(data);
		printf("failed to load texture at path: %s\n", path);
		exit(-1);
	}
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); // nearest neighbor minification
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); // linear interpolated magnification
	// set texture to data from jpg
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, (transparent) ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
}
