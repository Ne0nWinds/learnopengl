#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/call.h>

void resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0,0,width,height);
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

	GLFWwindow* window = glfwCreateWindow(800, 600,"",NULL,NULL);

	glewExperimental = GL_TRUE;

	if (!glewInit())
	{
		printf("Failed to load glew");
		return -1;
	}

	glViewport(0,0,800,600);
	glfwSetFramebufferSizeCallback(window, resize_callback);

	while (!glfwWindowShouldClose(window))
	{
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
