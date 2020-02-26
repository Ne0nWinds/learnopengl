#include <stdlib.h>
#include <stdio.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cglm/cglm.h>
#include <cglm/call.h>

unsigned int EBO ,VBO, VAO;
unsigned int vertexShader, fragmentShader, shaderProgram;

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
	printf("Not enough memory\n");
	exit(-1);
}

void resize_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0,0,width,height);
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

void attachShaders()
{
	shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);

	int success = 0;
	char infoLog[512];
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		fprintf(stderr, "Error linking to program: '%s'\n", infoLog);
		exit(-1);
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
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

	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("glew failed to init\n");
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	compileShader(&vertexShader, GL_VERTEX_SHADER, "./shaders/vertex.shader");
	compileShader(&fragmentShader, GL_FRAGMENT_SHADER, "./shaders/fragment.shader");

	attachShaders();

	float vertices[] = {
		0.5f,  0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f
	};
	unsigned int indices[] = {
		0, 1, 3, // triangle 1
		1, 2, 3  // triangle 2
	};

	printf("testing 123\n");

	// Allocate GPU Memory
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);

	// Start editing Vertex Array Object
	glBindVertexArray(VAO);

	// Copy vertices array to Vertex Buffer Object
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	// Copy index array to Element Buffer Object
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Set and enable aPos vec3 in shader
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	while (!glfwWindowShouldClose(window))
	{
		// Clear screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// Use program and draw square
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		// Uncomment to use wireframe Mode
		// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}
