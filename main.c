#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/call.h>

#include "./src/gl_functions.h"

unsigned int EBO ,VBO, VAO, lightVAO;
unsigned int vertexShader, fragmentShader, shaderProgram;
unsigned int lightVertexShader, lightFragmentShader, lightShaderProgram;

unsigned int windowWidth, windowHeight;

const double to_radians = M_PI / 180;
vec3 lightPos = { 1.2f, 1.0f, 2.0f };
float specularStrength = 0.5;

vec3 cameraPos = { 0.0f, 0.0f, 3.0f };
vec3 cameraFront = { 0.0f, 0.0f, -1.0f };
vec3 cameraUp = { 0.0f, 1.0f, 0.0f };
double pitch = 0;
double yaw = 1;
double lastX, lastY;
const float mouse_sensitivity = 0.05f;

float FOV = 45.0f;
float move_speed = 2.5f;

double lastFrame = 0.0f;
double deltaTime = 0.0f;
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
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		move_speed = 4.0f;
		FOV = 52.0f;
	} else {
		move_speed = 2.5f;
		FOV = 45.0f;
	}
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
	unsigned int diffuseMap, specularMap;
	loadTexture(&diffuseMap, "./textures/container2.png", true);
	loadTexture(&specularMap, "./textures/container2_specular.png", true);

	compileShader(&vertexShader, GL_VERTEX_SHADER, "./shaders/vertex.shader");
	compileShader(&fragmentShader, GL_FRAGMENT_SHADER, "./shaders/fragment.shader");
	attachShaders(&shaderProgram, vertexShader, fragmentShader);

	compileShader(&lightVertexShader, GL_VERTEX_SHADER, "./shaders/lightvertex.shader");
	compileShader(&lightFragmentShader, GL_FRAGMENT_SHADER, "./shaders/lightfragment.shader");
	attachShaders(&lightShaderProgram, lightVertexShader, lightFragmentShader);
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
	};

	float cubePositions[] = {
		0.0f,  0.0f,  0.0f,
		1.0f,  2.0f,  3.0f,
		1.9f,  2.5f,  5.0f,
		-1.0f,  2.0f,  2.0f,
	};


	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
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

	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window))
	{
		double currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glClearColor(0.06f, 0.06f, 0.06f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float alpha = 0;
		processInput(window);

		glUseProgram(shaderProgram);

		mat4 view;
		glm_mat4_identity(view);

		vec3 center; glm_vec3_add(cameraPos, cameraFront, center);
		glm_lookat(cameraPos, center, cameraUp, view);

		mat4 projection;
		glm_mat4_identity(projection);
		glm_perspective(to_radians * (FOV), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f, projection);

		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, *view);
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, *projection);

		vec3 coral = { 1.0f, 0.5f, 0.31f };
		vec3 lightColor = { 1.0f, 1.0f, 1.0f };
		glUniform3fv(glGetUniformLocation(shaderProgram, "lightColor"), 1, lightColor);
		glUniform3fv(glGetUniformLocation(shaderProgram, "viewPos"), 1, cameraPos);
		glUniform1f(glGetUniformLocation(shaderProgram, "material.shiniess"), 12.0f);
		glUniform1f(glGetUniformLocation(shaderProgram, "material.diffuse"), 0);
		glUniform1f(glGetUniformLocation(shaderProgram, "material.specular"), 1);
		vec3 ambient = { 0.15f, 0.15f, 0.15f };
		vec3 light_diffuse = { 0.8f, 0.8f, 0.8f };
		glm_vec3_mul(lightColor, light_diffuse, light_diffuse);
		glm_vec3_mul(lightColor, ambient, ambient);
		vec3 light_specular = { 1.0f, 1.0f, 1.0f };
		vec3 lightPos = { 2.0f * sin(currentFrame), 0.0f, 2.0f * cos(currentFrame) };
		glUniform3fv(glGetUniformLocation(shaderProgram, "light.ambient"), 1, ambient);
		glUniform3fv(glGetUniformLocation(shaderProgram, "light.diffuse"), 1, light_diffuse);
		glUniform3fv(glGetUniformLocation(shaderProgram, "light.specular"), 1, light_specular);
		glUniform3fv(glGetUniformLocation(shaderProgram, "light.position"), 1, lightPos);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		glBindVertexArray(VAO);
		for(unsigned int i = 0; i < sizeof(cubePositions) / sizeof(float); i+=3)
		{
			mat4 model;
			glm_mat4_identity(model);
			vec3 translation = {
				cubePositions[i],
				cubePositions[i+1],
				cubePositions[i+2],
			};
			glm_translate(model, translation);
			glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, *model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		glUseProgram(lightShaderProgram);
		mat4 model;
		glm_mat4_identity(model);
		glm_translate(model, lightPos);
		vec3 lightSize = { 0.5f, 0.5f, 0.5f };
		glm_scale(model, lightSize);
		glUniform3fv(glGetUniformLocation(lightShaderProgram, "lightColor"), 1, lightColor);
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "model"), 1, GL_FALSE, *model);
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "view"), 1, GL_FALSE, *view);
		glUniformMatrix4fv(glGetUniformLocation(lightShaderProgram, "projection"), 1, GL_FALSE, *projection);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
