#include <iostream>
#include <cmath>
// GLEW //
#define GLEW_STATIC
#include <GL/glew.h>
// GLFW //
#include <GLFW/glfw3.h>
// SOIL //
#include <SOIL/SOIL.h>
// GLM //
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// Other includes //
#include "Shader.h"
#include "Model.h"

//Function Prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void do_movement();
//Dimension of Window
const GLuint WIDTH = 800, HEIGHT = 600;

//Used for camera movement - consistent framerates across all PCs
GLfloat deltaTime = 0.0f; //Time between current frame and last frame
GLfloat lastFrame = 0.0f; //Time of last frame

//Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
bool keys[1024];

/*
//Camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, 0.0f);

//Camera Direction
glm::vec3 cameraDirection = glm::normalize(cameraPos - cameraTarget); //z axis vector

//Right Axis
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f); //Vector pointing upwards in world space
glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraDirection)); //x axis vector
//Cross product of up and cameraDirection vector thus resulting in vector perpendicular to both (pointing to x axis)

//Up Axis
glm::vec3 cameraUp = glm::cross(cameraDirection, cameraRight); //y axis vector
//cross product of x and z axis

*/

int main()
// Instantiate the GLFW Window //

{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); //What options we want to configure
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //Integer that sets value of our option
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Using Core Profile of OpenGL instead of Immediate Mode
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	/* GLFW's Create Window function. Arg 1, 2 = Width, Height. Arg 3 = Window name
	nullptr = ignore */
	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "EPQ", nullptr, nullptr);

	if (window == nullptr)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate(); //GLFW ends
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetKeyCallback(window, key_callback);

	glewExperimental = GL_TRUE; //Ensures GLEW uses more modern techniques to manage OpenGL functionality
	glewInit();
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Failed to initialise GLEW" << std::endl;
		return -1;
	}

	glViewport(0, 0, WIDTH, HEIGHT); //Specifies size of the rendering window (different from GLFW Window)

	glEnable(GL_DEPTH_TEST); //Enable Z buffer

							 // Build and compile the shader program //
	//Shader ourShader("D:/Documents/Visual Studio 2015/Projects/newEPQ/newEPQ/vertex.txt", "D:/Documents/Visual Studio 2015/Projects/newEPQ/newEPQ/fragment.txt");
	Shader ourShader("vertex.txt", "fragment.txt");
	Model ourModel("monkey/monkey.obj");

	//Model ourModel("nanosuit/nanosuit.obj");
	//Model ourModel("D:/Documents/Visual Studio 2015/Projects/newEPQ/newEPQ/monkey/monkey.obj");
	//Model ourModel("teapot/teapot.obj");
	//Model ourModel("head/head.obj");

	// Game Loop //
	/* Keeps drawing images and handling user input until program is told to stop */

	while (!glfwWindowShouldClose(window)) //checks if GLFW is told to close every loop iteration
	{
		//Calculate deltatime of current frame
		GLfloat currentframe = glfwGetTime();
		deltaTime = currentframe - lastFrame;
		lastFrame = currentframe;
		
		
		glfwPollEvents(); //checks if any events are triggered (e.g. mouse input)
		do_movement();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);  //This colour fills the screen whenever buffer is cleared
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //clear screen's colour buffer



		// Draw Shape //
		ourShader.Use();

		// Create Transformations //

		glm::mat4 view; //View space matrix
		glm::mat4 projection; //Perspective projection matrix

							  // Camera Transformations //

		view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp); //Translate z axis in reverse direction of where we want to move
																		  /*
																		  1. Matrix to translate
																		  2. Translation vector
																		  */
		projection = glm::perspective(glm::radians(45.0f), (GLfloat)WIDTH / (GLfloat)HEIGHT, 0.1f, 100.0f);
		/*
		1. FoV
		2. Aspect ratio (sets the height of the frustum)
		3. Near plane
		4. Far plane
		*/


		glm::mat4 model;
		model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f)); // Translate it down a bit so it's at the center of the scene
		model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5));	// It's a bit too big for our scene, so scale it down
		//model = glm::translate(model, glm::vec3(0.0f, -1.75f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
		model = glm::rotate(model, GLfloat(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(ourShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(model));
		ourModel.Draw(ourShader);


		// Find Matrix Uniform location and set matrix //
		GLuint modelLoc = glGetUniformLocation(ourShader.Program, "model");
		GLuint viewLoc = glGetUniformLocation(ourShader.Program, "view");
		GLuint projectionLoc = glGetUniformLocation(ourShader.Program, "projection");

		// Pass to shaders //

		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
		/*
		1. Uniform Location
		2. How many matrices to send
		3. transpose matrix
		4. actual matrix data stored in the form of value_ptr
		*/


		
		


		glBindVertexArray(0);

		glfwSwapBuffers(window); //display the other Color buffer as an output
	}
	//glDeleteVertexArrays(1, &VAO);
	//glDeleteBuffers(1, &VBO);
	glfwTerminate();
	return 0;
}



// GLFW callback function //
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	std::cout << key << std::endl;
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) //when user presses escape key, WindowShouldClose = true
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;


	
}

void do_movement()
{
	GLfloat cameraSpeed = 5.0f * deltaTime;
	if (keys[GLFW_KEY_W])
		cameraPos += cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_S])
		cameraPos -= cameraSpeed * cameraFront;
	if (keys[GLFW_KEY_A])
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (keys[GLFW_KEY_D])
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	

}
