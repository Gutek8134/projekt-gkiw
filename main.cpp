// libglew-dev
#include <GL/glew.h>
// libglm-dev
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
// libglfw3-dev
#include <GLFW/glfw3.h>

#include <stdlib.h>
#include <stdio.h>

float speed_x = 0; //[radians/s]
float speed_y = 0; //[radians/s]

// Error processing callback procedure
void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

// Initialization code procedure
void initOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, at the program start************
}

// Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, after the main loop ends************
}

// Drawing procedure
void drawScene(GLFWwindow *window, float angle)
{
    //************Place any code here that draws something inside the window******************l
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);
}

int main(void)
{
    GLFWwindow *window; // Pointer to object that represents the application window

    glfwSetErrorCallback(error_callback); // Register error processing callback procedure

    if (!glfwInit())
    { // Initialize GLFW library
        fprintf(stderr, "Can't initialize GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(1280, 720, "Ship", NULL, NULL); // Create a window 1280pxx720px titled "Ship" and an OpenGL context associated with it.

    if (!window) // If no window is opened then close the program
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window); // Since this moment OpenGL context corresponding to the window is active and all OpenGL calls will refer to this context.
    glfwSwapInterval(1);            // During vsync wait for the first refresh

    GLenum err;
    if ((err = glewInit()) != GLEW_OK)
    { // Initialize GLEW library
        fprintf(stderr, "Can't initialize GLEW: %s\n", glewGetErrorString(err));
        exit(EXIT_FAILURE);
    }

    initOpenGLProgram(window); // Call initialization procedure

    glfwSetTime(0);
    // Main application loop
    while (!glfwWindowShouldClose(window)) // As long as the window shouldnt be closed yet...
    {
        glfwSetTime(0);
        drawScene(window, 0.f); // Execute drawing procedure
        glfwPollEvents();       // Process callback procedures corresponding to the events that took place up to now
    }
    freeOpenGLProgram(window);

    glfwDestroyWindow(window); // Delete OpenGL context and the window.
    glfwTerminate();           // Free GLFW resources
    exit(EXIT_SUCCESS);
}