#include <iostream>         // Allow for input/output
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library

// GLM Libraries
#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp> 

using namespace std;

int width, height;			// window variables
bool perspective = true;	// boolean to change between perspective and orthographic

// Input fucntions 
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

void initCamera();			// Function to reset camera
glm::mat4 viewMatrix = glm::mat4(1.0f);		// Delcare View matrix
GLfloat fov = 45.0f;		// Declare and nitialize field of view

// Define camera attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, -3.0f, 10.0f);
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));

// Pitch, yaw, 
GLfloat yaw = -90.0f, pitch = 0.0f;
// Variables for scroll and cursor
GLfloat cameraMovement = 10.0f;
GLfloat cameraSpeed = 2.5f;
GLfloat delataTime = 0.0f, lastFrame = 0.0f; // Variables to ensure application runs the same on all hardware
GLfloat lastX = 320, lastY = 240, xChange, yChange;
bool firstMouseMove = true; // Detect initial mouse movement


// Create and compile shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	GLuint shaderID = glCreateShader(shaderType);	// Create shader object
	const char* src = source.c_str();

	glShaderSource(shaderID, 1, &src, nullptr);		// Attach source code to shader object
	glCompileShader(shaderID);						// Compile shader
	return shaderID;								// Return shader ID

}

// Create program object to link shader objects 
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);
	GLuint shaderProgram = glCreateProgram();	// Create program object

	// Attach compiled vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create final executable shader program
	glLinkProgram(shaderProgram);

	// Delete vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);
	return shaderProgram;	// Return shader Program

}

int main(void)
{
	width = 640; height = 480;	// Set values for screen dimensions
	GLFWwindow* window;		// Declare new window object

	// Initialize glfw library 
	if (!glfwInit())
	{
		return -1;
	}

	// Create GLFW window
	window = glfwCreateWindow(width, height, "3D Scene by Paul K.", NULL, NULL);

	// Terminate program if window is not created
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

	// Make context current for calling thread
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
	{
		cout << "Error!" << endl;
	}

	glEnable(GL_DEPTH_TEST);	// Allows for depth comparisons and to update the depth buffer

	// Wireframe
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	// Define vertex data for triangles in a floating point array for PLANE
	GLfloat planeVertices[] = {

		-5.0, -5.0, -5.0,	1.0, 0.0, 0.0,
		 5.0, -5.0, -5.0,	1.0, 0.0, 0.0,
		 5.0, -5.0, 5.0,	1.0, 0.0, 0.0,
		 5.0, -5.0, 5.0,	1.0, 0.0, 0.0,
		-5.0, -5.0, 5.0,	1.0, 0.0, 0.0,
		-5.0, -5.0, -5.0,	1.0, 0.0, 0.0
	};


	// Create variables for PLANE reference
	GLuint VBO1,VAO1;

	// Create new Vertex Buffer Object, Vertex Array Object, and Element Buffer Object
	glGenVertexArrays(1, &VAO1);
	glGenBuffers(1, &VBO1);	// (# of buffers, where to store id for buffer when created)
	glBindVertexArray(VAO1);
	glBindBuffer(GL_ARRAY_BUFFER, VBO1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);

	// Position attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Define vertex data for triangles in a floating point array for Almond Milk base cube
	GLfloat milkCubeVertices[] = {

		-2.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -2.0,  0.0,		0.0, 1.0, 0.0,		// Top of Almond Milk cube
		-2.0, -2.0,  0.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -2.0,  0.0,		0.0, 1.0, 0.0,
		
		-2.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, 0.0,		0.0, 1.0, 0.0,		// Bottom of Almond Milk cube
		-2.0, -5.0, 0.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, 0.0,		0.0, 1.0, 0.0,

		-2.0, -2.0, 0.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, 0.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, 0.0,		0.0, 1.0, 0.0,		// Front of Almond Milk cube
		 0.0, -5.0, 0.0,		0.0, 1.0, 0.0,
		-2.0, -2.0, 0.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, 0.0,		0.0, 1.0, 0.0,

		-2.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 0.0,		// Back of Almond Milk cube
		 0.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		
		-2.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -2.0, 0.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, 0.0,		0.0, 1.0, 0.0,		// Left side of Almond Milk cube
		-2.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, -2.0,		0.0, 1.0, 0.0,
		-2.0, -5.0, 0.0,		0.0, 1.0, 0.0,

		 0.0, -2.0,  0.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, -2.0,		0.0, 1.0, 0.0, 
		 0.0, -5.0,  0.0,		0.0, 1.0, 0.0,		// Right side of Almond Milk cube
		 0.0, -2.0,  0.0,		0.0, 1.0, 0.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 0.0,
		 0.0, -5.0, -2.0,		0.0, 1.0, 0.0
	};

	// Create variables for Almond Milk base cube reference
	GLuint VBO2, VAO2;

	// Create new Vertex Buffer Object, Vertex Array Object, and Element Buffer Object
	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2); // (# of buffers, where to store id for buffer when created)
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(milkCubeVertices), milkCubeVertices, GL_STATIC_DRAW);

	// Position attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Define vertex data for triangles in a floating point array for Almond Milk top pyramids
	GLfloat milkCPyramidVertices[] = {

		-2.0, -2.0, -2.0,		0.0, 0.0, 1.0,
		-1.0, -2.0,  0.0,		0.0, 0.0, 1.0,
		-2.0, -2.0,  0.0,		0.0, 0.0, 1.0,		// Bottom of left Almond Milk pyramid NOT SEEEN
		-2.0, -2.0, -2.0,		0.0, 0.0, 1.0,
		-1.0, -2.0,  0.0,		0.0, 0.0, 1.0,
		-1.0, -2.0, -2.0,		0.0, 0.0, 1.0,

		-2.0, -2.0, -2.0,		1.0, 0.0, 0.0,
		-2.0, -2.0,  0.0,		1.0, 0.0, 0.0,
		-1.5, -0.5, -1.0,		1.0, 0.0, 0.0,		// Left side of left Almond Milk pyramid RED

		-2.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		-1.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		-1.5, -0.5, -1.0,		0.0, 1.0, 1.0,		// Back side of left Almond Milk pyramid CYAN

		-1.0, -2.0, -2.0,		0.0, 0.0, 1.0,
		-1.0, -2.0,  0.0,		0.0, 0.0, 1.0,
		-1.5, -0.5, -1.0,		0.0, 0.0, 1.0,		// Right side of left Almond Milk pyramid NOT SEEEN

		-2.0, -2.0, 0.0,		1.0, 0.0, 1.0,
		-1.0, -2.0,  0.0,		1.0, 0.0, 1.0,
		-1.5, -0.5, -1.0,		1.0, 0.0, 1.0,		// Front side of left Almond Milk pyramid PURPLE

		-1.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		-1.0, -2.0,  0.0,		0.0, 1.0, 1.0, 
		 0.0, -2.0,  0.0,		0.0, 1.0, 1.0,		// Bottom of right Almond Milk pyramid NOT SEEEN
		-1.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		 0.0, -2.0,  0.0,		0.0, 1.0, 1.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 1.0,

		-1.0, -2.0,  0.0,		0.0, 1.0, 1.0,
		-1.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		-0.5, -0.5, -1.0,		0.0, 1.0, 1.0,		// Left side of right Almond Milk pyramid NOT SEEEN

		-1.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		 0.0, -2.0, -2.0,		0.0, 1.0, 1.0,
		-0.5, -0.5, -1.0,		0.0, 1.0, 1.0,		// Back side of right Almond Milk pyramid CYAN

		 0.0, -2.0, -2.0,		1.0, 0.0, 0.0,
		 0.0, -2.0,  0.0,		1.0, 0.0, 0.0,
		-0.5, -0.5, -1.0,		1.0, 0.0, 0.0,		// Right side of right Almond Milk pyramid RED

		 0.0, -2.0,  0.0,		1.0, 0.0, 1.0,
		-1.0, -2.0,  0.0,		1.0, 0.0, 1.0,
		-0.5, -0.5, -1.0,		1.0, 0.0, 1.0,		// Front side of right Almond Milk pyramid PURPLE

		-1.0, -2.0,  0.0,		1.0, 0.0, 1.0,
		-1.5, -0.5, -1.0,		1.0, 0.0, 1.0,
		-0.5, -0.5, -1.0,		1.0, 0.0, 1.0,		// Front triangle connecting both pyramids PURPLE

		 -1, -2.0,  -2.0,		0.0, 1.0, 1.0,
		-1.5, -0.5, -1.0,		0.0, 1.0, 1.0,
		-0.5, -0.5, -1.0,		0.0, 1.0, 1.0		// Back triangle connecting both pyramids CYAN
	
	};

	// Create variables for Almond Milk top reference
	GLuint VBO3, VAO3;

	// Create new Vertex Buffer Object, Vertex Array Object, and Element Buffer Object
	glGenVertexArrays(1, &VAO3);
	glGenBuffers(1, &VBO3); // (# of buffers, where to store id for buffer when created)
	glBindVertexArray(VAO3);
	glBindBuffer(GL_ARRAY_BUFFER, VBO3);
	glBufferData(GL_ARRAY_BUFFER, sizeof(milkCPyramidVertices), milkCPyramidVertices, GL_STATIC_DRAW);

	// Position attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Define vertex data for triangles in a floating point array for Donut box 
	GLfloat donutBoxVertices[] =
	{
		1.0, -5.0, 0.0,		0.5, 0.5, 0.5,
		2.5, -5.0, -1.5,	0.5, 0.5, 0.5,
		3.0, -5.0, 2.0,		0.5, 0.5, 0.5,
		3.0, -5.0, 2.0,		0.5, 0.5, 0.5,		// Bottom of donut box cube
		4.5, -5.0, 0.5,		0.5, 0.5, 0.5,
		2.5, -5.0, -1.5,	0.5, 0.5, 0.5,

		1.0, -4.0, 0.0,		0.5, 0.5, 0.5,
		2.5, -4.0, -1.5,	0.5, 0.5, 0.5,
		3.0, -4.0, 2.0,		0.5, 0.5, 0.5,
		3.0, -4.0, 2.0,		0.5, 0.5, 0.5,		// Top of donut box cube
		4.5, -4.0, 0.5,		0.5, 0.5, 0.5,
		2.5, -4.0, -1.5,	0.5, 0.5, 0.5,

		1.0, -5.0, 0.0,		0.5, 0.5, 0.5,
		1.0, -4.0, 0.0,		0.5, 0.5, 0.5,
		3.0, -5.0, 2.0,		0.5, 0.5, 0.5,		// Front of donut box cube
		3.0, -5.0, 2.0,		0.5, 0.5, 0.5,
		1.0, -4.0, 0.0,		0.5, 0.5, 0.5,
		3.0, -4.0, 2.0,		0.5, 0.5, 0.5,

		3.0, -5.0, 2.0,		0.5, 0.5, 0.5,
		3.0, -4.0, 2.0,		0.5, 0.5, 0.5,
		4.5, -5.0, 0.5,		0.5, 0.5, 0.5,		// Right side of donut box cube
		4.5, -5.0, 0.5,		0.5, 0.5, 0.5,
		3.0, -4.0, 2.0,		0.5, 0.5, 0.5,
		4.5, -4.0, 0.5,		0.5, 0.5, 0.5,

		1.0, -5.0, 0.0,		0.5, 0.5, 0.5,
		1.0, -4.0, 0.0,		0.5, 0.5, 0.5,
		2.5, -5.0, -1.5,	0.5, 0.5, 0.5,		// Left side of donut box cube
		2.5, -5.0, -1.5,	0.5, 0.5, 0.5,
		1.0, -4.0, 0.0,		0.5, 0.5, 0.5,
		2.5, -4.0, -1.5,	0.5, 0.5, 0.5,

		2.5, -5.0, -1.5,	0.5, 0.5, 0.5,
		2.5, -4.0, -1.5,	0.5, 0.5, 0.5,
		4.5, -5.0, 0.5,		0.5, 0.5, 0.5,	// Back side of donut box cube
		4.5, -5.0, 0.5,		0.5, 0.5, 0.5,
		2.5, -4.0, -1.5,	0.5, 0.5, 0.5,
		4.5, -4.0, 0.5,		0.5, 0.5, 0.5
	};

	// Create variables for Donut box reference
	GLuint VBO4, VAO4;

	// Create new Vertex Buffer Object, Vertex Array Object, and Element Buffer Object
	glGenVertexArrays(1, &VAO4);
	glGenBuffers(1, &VBO4); // (# of buffers, where to store id for buffer when created)
	glBindVertexArray(VAO4);
	glBindBuffer(GL_ARRAY_BUFFER, VBO4);
	glBufferData(GL_ARRAY_BUFFER, sizeof(donutBoxVertices), donutBoxVertices, GL_STATIC_DRAW);

	// Position attributes
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// Unbind VOA
	glBindVertexArray(0);

	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"						// Version of OpenGL
		"layout(location = 0) in vec4 vPosition;"	// Specify location of position attributes
		"layout(location = 1) in vec4 aColor;"		// Specify location of color attributes
		"out vec4 oColor;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n" // Entry point for shader
		"{\n"
		"gl_Position = projection * view * model * vPosition;"		// Output position coordinates
		"oColor = aColor;"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"						// Version of OpenGL
		"in vec4 oColor;"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = oColor;"						// Specify colors
		"}\n";

	// Create shader program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);

	// Render loop (infinite loop until user closes window)
	while (!glfwWindowShouldClose(window))
	{
		// Set delta time
		GLfloat currentFrame = glfwGetTime();
		delataTime = currentFrame - lastFrame;  // Ensure we are transforming at consistent rate
		lastFrame = currentFrame;

		processInput(window);

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		// Set background color
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Declare identity matrix
		glm::mat4 modelMatrix = glm::mat4(1.0f);
		glm::mat4 projectionMatrix = glm::mat4(1.0f);

		// Use executable shader program and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes

		// lookAt functin used to create view matrix that transforms all world coordinates to view space
		viewMatrix = glm::lookAt(cameraPosition, cameraPosition + cameraFront, cameraUp);

		// Conditional loop allows user to change view of scene between orthographic (2D) and perspective (3D) views
		if (perspective)
		{
			projectionMatrix = glm::perspective(glm::radians(fov), (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
		}

		else{
			float scale = 100;
			projectionMatrix = glm::ortho(-((float)width / scale), (float)width / scale, -(float)height / scale, ((float)height / scale), -50.0f, 50.0f);
		}

		// Select uniform shader and variable
		GLuint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLuint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLuint projectionLoc = glGetUniformLocation(shaderProgram, "projection");

		// Pass transform to shader
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

		// Draw Plane
		glBindVertexArray(VAO1);
		modelMatrix = glm::mat4(1.0f); // Declare identity matrix
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f)); // Translate on x axis
		modelMatrix = glm::rotate(modelMatrix, glm::degrees(0.0f), glm::vec3(1.0f, 0.3f, 0.5f)); // Rotate along y axis
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// Draw Almond milk base
		glBindVertexArray(VAO2);
		modelMatrix = glm::mat4(1.0f); // Declare identity matrix
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f)); 
		glDrawArrays(GL_TRIANGLES, 0, 36);

		// Draw Almond milk top
		glBindVertexArray(VAO3);
		modelMatrix = glm::mat4(1.0f); // Declare identity matrix
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f)); 
		glDrawArrays(GL_TRIANGLES, 0, 42);

		// Draw donut box
		glBindVertexArray(VAO4);
		modelMatrix = glm::mat4(1.0f); // Declare identity matrix
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 5.0f, 0.0f));
		glDrawArrays(GL_TRIANGLES, 0, 36);
		
		// Deactivate VAO
		glBindVertexArray(0);

		// Deactivate program object
		glUseProgram(0);

		// Swap front and back buffers of window
		glfwSwapBuffers(window);

		// Process events
		glfwPollEvents();
	}

	// Delete Vertex Array Objects and Vertex Buffer Objects
	glDeleteVertexArrays(1, &VAO1);
	glDeleteBuffers(1, &VBO1);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteVertexArrays(1, &VAO3);
	glDeleteBuffers(1, &VBO3);
	glDeleteVertexArrays(1, &VAO4);
	glDeleteBuffers(1, &VBO4);

	// End program
	glfwTerminate();
	return 0;
}

// Function to process user keyboard input
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		// Exit application if escape key pressed	
	{
		glfwSetWindowShouldClose(window, true);					
	}
	
	cameraSpeed = cameraMovement * delataTime;					// Calculate camera speed based on scroll

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)			// If 'W' pressed, move camera forward (toward object)	
	{
		cameraPosition += cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)			// If 'S' pressed, move camera backward (away from object)	
	{
		cameraPosition -= cameraSpeed * cameraFront;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			// If 'A' pressed, move camera left	
	{
		cameraPosition -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)			// If 'D' pressed, move camera right	
	{
		cameraPosition += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	}
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)			// If 'Q' pressed, move camera down	
	{
		cameraPosition -= cameraSpeed * cameraUp;
	}
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)			// If 'E' pressed, move camera up	
	{
		cameraPosition += cameraSpeed * cameraUp;
	}
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)			// If "P" key pressed, change perspective
	{
		perspective = !perspective;
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)			// If 'F' pressed, call function to reset camera
	{
		initCamera();
	}
}

// Control speed at which camera moves with scroll; Adjust the speed of the movement
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// Clamp cameraMovement
	if (cameraMovement >= 1.0f && cameraMovement <= 55.0f)
	{
		cameraMovement -= yoffset;
	}

	// Default cameraMovement
	if (cameraMovement < 1.0f)
	{
		cameraMovement = 1.0f;
	}

	if (cameraMovement > 55.0f)
	{
		cameraMovement = 55.0f;
	}
}

// Allows to change the orientation of the camera
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	// Update initial mouse positions if this is the first move from mouse
	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	// Calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Lessen sensitivity of mouse movement
	GLfloat sensitivity = 0.1f;
	xChange *= sensitivity;
	yChange *= sensitivity;

	// Add ofset values to global yaw and pitch
	yaw += xChange;
	pitch += yChange;

	// Prevent screen from flipping
	if (pitch > 89.0f)
	{
		pitch = 89.0f;
	}
	if (pitch < -89.0f)
	{
		pitch = -89.0f;
	}

	// Calculate actual direction vector to contain rotations from mouse movement
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(front);
}

// Function with coordinates to reset camera to look at scene
void initCamera()
{
	cameraPosition = glm::vec3(0.0f, -3.0f, 15.0f);
	target = glm::vec3(0.0f, 0.0f, 0.0f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
}