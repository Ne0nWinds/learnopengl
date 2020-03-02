#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/call.h>

#define STB_IMAGE_IMPLEMENTATION
#include "./src/stb_image.h"

unsigned int EBO ,VBO, VAO;
unsigned int vertexShader, fragmentShader, shaderProgram;

unsigned int windowWidth, windowHeight;

const double to_radians = M_PI / 180;

vec3 cameraPos = { 0.0f, 0.0f, 3.0f };
vec3 cameraFront = { 0.0f, 0.0f, -1.0f };
vec3 cameraUp = { 0.0f, 1.0f, 0.0f };
double pitch = 0;
double yaw = 1;
double lastX, lastY;
const float mouse_sensitivity = 0.05f;
float FOV = 45.0f;
float move_speed = 2.5f;

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

double lastFrame = 0.0f;
double deltaTime = 0.0f;
void setSprint(bool active) {
	if (active) {
		move_speed = 3.5f;
		FOV = glm_lerp(FOV, 52.0f, 0.15f);
	} else {
		move_speed = 2.5f;
		FOV = glm_lerp(FOV, 45.0f, 0.3f);
	}
}
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	float cameraSpeed = move_speed * deltaTime;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
	{
		vec3 add; glm_vec3_scale(cameraFront, cameraSpeed, add);
		glm_vec3_add(cameraPos, add, cameraPos);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
	{
		vec3 sub; glm_vec3_scale(cameraFront, cameraSpeed, sub);
		glm_vec3_sub(cameraPos, sub, cameraPos);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		vec3 cross; glm_cross(cameraFront, cameraUp, cross);
		glm_normalize(cross);
		glm_vec3_scale(cross, cameraSpeed, cross);
		glm_vec3_sub(cameraPos, cross, cameraPos);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		vec3 cross; glm_cross(cameraFront, cameraUp, cross);
		glm_normalize(cross);
		glm_vec3_scale(cross, cameraSpeed, cross);
		glm_vec3_add(cameraPos, cross, cameraPos);
	}
	setSprint(glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS);
}
void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	double xoffset = (xpos - lastX) * mouse_sensitivity;
	double yoffset = (lastY - ypos) * mouse_sensitivity;
	lastX = xpos;
	lastY = ypos;
	yaw += xoffset;
	yaw = fmod(yaw, 360.0f);
	pitch += yoffset;
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;
	vec3 direction = {
		cos(to_radians * yaw) * cos(to_radians * pitch),
		to_radians * pitch,
		sin(to_radians * yaw) * cos(to_radians * pitch)
	};
	glm_normalize(direction);
	glm_vec3_copy(direction, cameraFront);
};

void init_mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
	lastX = xpos;
	lastY = ypos;
	glfwSetCursorPosCallback(window, mouse_callback);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
	windowWidth = width;
	windowHeight = height;
	glViewport(0,0,width,height);
	glfwSetCursorPosCallback(window, init_mouse_callback);
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

int main(void)
{
	if (!glfwInit())
	{
		printf("Failed to load glfw\n");
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(640, 480,"",NULL,NULL);
	glfwMakeContextCurrent(window);

	glViewport(0,0,640,480);
	glfwSetFramebufferSizeCallback(window, resize_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, init_mouse_callback);

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("glew failed to init\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}
	glEnable(GL_DEPTH_TEST);

	// allocate memory for texture in OpenGL
	unsigned int texture1, texture2;
	loadTexture(&texture1, "./textures/container.jpg", false);

	compileShader(&vertexShader, GL_VERTEX_SHADER, "./shaders/vertex.shader");
	compileShader(&fragmentShader, GL_FRAGMENT_SHADER, "./shaders/fragment.shader");

	attachShaders(&shaderProgram, vertexShader, fragmentShader);

	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};

	float cubePositions[] = {
		0.0f,  0.0f,  0.0f,
		2.0f,  5.0f, -15.0f,
		-1.5f, -2.2f, -2.5f,
		-3.8f, -2.0f, -12.3f,
		2.4f, -0.4f, -3.5f,
		-1.7f,  3.0f, -7.5f,
		1.3f, -2.0f, -2.5f,
		1.5f,  2.0f, -2.5f,
		1.5f,  0.2f, -1.5f,
		-1.3f,  1.0f, -1.5f,
	};

	// Allocate GPU Memory
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	// Start editing Vertex Array Object
	glBindVertexArray(VAO);

	// Copy vertices array to Vertex Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// glVertexAttribPointer(
	// 		location in shader
	// 		number of data points needed for attribute (3 for rgb color)
	// 		type (using floats for all)
	// 		normalize values
	// 		byte offset to between attributes
	// 		byte offset to first attribute
	// 	)
	// Set and enable aPos vec3 in shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Set and enable aTextureCoord vec2 in shader
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	glUseProgram(shaderProgram);
	glUniform1i(glGetUniformLocation(shaderProgram, "texture1"), 0);
	mat4 model;
	glm_mat4_identity(model);
	while (!glfwWindowShouldClose(window))
	{

		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		float alpha = 0;
		processInput(window);

		vec3 rotation =  {-1.0f, 1.0f, 0.0f};
		glm_rotate(model, -1.0f * to_radians, rotation);

		mat4 view;
		glm_mat4_identity(view);

		vec3 center; glm_vec3_add(cameraPos, cameraFront, center);
		glm_lookat(cameraPos, center, cameraUp, view);

		mat4 projection;
		glm_mat4_identity(projection);
		glm_perspective(to_radians * (FOV), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f, projection);

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, *view);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, *projection);
		glUniform1f(glGetUniformLocation(shaderProgram, "alpha"),alpha);
		// Uncomment line below to use wireframe Mode
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture2);
		glBindVertexArray(VAO);
		// glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		for(unsigned int i = 0; i < 30; i+=3)
		{
			mat4 model;
			glm_mat4_identity(model);
			vec3 translation = {
				cubePositions[i],
				cubePositions[i+1],
				cubePositions[i+2],
			};
			glm_translate(model, translation);
			vec3 rotation = {1.0f, 0.3f, 0.5f};
			glm_rotate(model, to_radians * 20.0f * i + currentFrame / 5, rotation);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, *model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
