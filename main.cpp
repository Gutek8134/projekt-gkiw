#define GLM_FORCE_RADIANS

// libglew-dev
#include <GL/glew.h>
// libglfw3-dev
#include <GLFW/glfw3.h>
// libglm-dev
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

// libassimp-dev
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <vector>

#define PI 3.14f
#define TAU 6.28f
#include "shaderprogram.h"
#include "myCube.h"
#include "mesh.h"

#define sky_color 0, 0.4f, 0.8f, 1
#define water_color 0, 0.3f, 1, 1
#define MAX_TIME 255

float speed_x = 0; //[radians/s]
float speed_y = 0; //[radians/s]
float wheel_speed = TAU / 8;

std::vector<Mesh *> meshes;

bool load_scene(const char *path)
{
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return false;
    }

    for (int i = 0; i < scene->mNumMeshes; ++i)
    {
        aiMesh *mesh = scene->mMeshes[i];
        meshes.push_back(new Mesh(mesh, scene));
    }

    return true;
}

glm::mat4 rotate_around(glm::mat4 m, glm::vec3 pivot, float angle, glm::vec3 axis)
{
    m = glm::translate(m, -pivot);
    m = glm::rotate(m, angle, axis);
    m = glm::translate(m, pivot);
    return m;
}

// Error processing callback procedure
void error_callback(int error, const char *description)
{
    fputs(description, stderr);
}

void key_callback(
    GLFWwindow *window,
    int key,
    int scancode,
    int action,
    int mod)
{
    if (action == GLFW_PRESS)
    {
        if (key == GLFW_KEY_LEFT)
        {
            speed_y = -PI;
        }
        if (key == GLFW_KEY_RIGHT)
        {
            speed_y = PI;
        }
        if (key == GLFW_KEY_UP)
        {
            speed_x = -PI;
        }
        if (key == GLFW_KEY_DOWN)
        {
            speed_x = PI;
        }
    }
    if (action == GLFW_RELEASE)
    {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT)
        {
            speed_y = 0;
        }
        if (key == GLFW_KEY_UP || key == GLFW_KEY_DOWN)
        {
            speed_x = 0;
        }
    }
}

ShaderProgram *Colored, *Textured, *LambertTextured;

// Initialization code procedure
void initOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, at the program start************
    Colored = new ShaderProgram("v_colored.glsl", "f_colored.glsl");
    Textured = new ShaderProgram("v_textured.glsl", "f_textured.glsl");
    LambertTextured = new ShaderProgram("v_lamberttextured.glsl", "f_lamberttextured.glsl");
    glClearColor(sky_color); // Set color buffer clear color
    glEnable(GL_DEPTH_TEST); // Turn on pixel depth test based on depth buffer
    glfwSetKeyCallback(window, key_callback);
    load_scene("statek.obj");
}

// Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, after the main loop ends************
    delete Colored, Textured, LambertTextured;
    for (Mesh *m : meshes)
    {
        delete m;
    }
    meshes.clear();
}

void drawWater(ShaderProgram *shader, glm::mat4 P, glm::mat4 V, glm::mat4 M)
{
    float vertices[] = {
        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,

        -1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        -1.0f,
        -1.0f,
        1.0f,
        1.0f,
        1.0f,
        -1.0f,
        1.0f,
        1.0f,
    },
          colors[] = {
              water_color,
              water_color,
              water_color,
              water_color,
              water_color,
              water_color,
          };

    shader->use();

    glUniformMatrix4fv(shader->getUniformLocation("P"), 1, false, glm::value_ptr(P));
    glUniformMatrix4fv(shader->getUniformLocation("V"), 1, false, glm::value_ptr(V));
    glUniformMatrix4fv(shader->getUniformLocation("M"), 1, false, glm::value_ptr(M));

    glEnableVertexAttribArray(shader->getAttributeLocation("vertex"));
    glVertexAttribPointer(shader->getAttributeLocation("vertex"), 4, GL_FLOAT, false, 0, vertices);

    glEnableVertexAttribArray(shader->getAttributeLocation("color"));
    glVertexAttribPointer(shader->getAttributeLocation("color"), 4, GL_FLOAT, false, 0, colors);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(shader->getAttributeLocation("vertex"));
    glDisableVertexAttribArray(shader->getAttributeLocation("color"));
}

// Drawing procedure
void drawScene(GLFWwindow *window, float angle_x, float angle_y, float wheel_angle, float time)
{
    //************Place any code here that draws something inside the window******************l
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

    glm::mat4 root_model_matrix = glm::mat4(1.0f);                                                                      // Initialize model matrix with abn identity matrix
    root_model_matrix = glm::rotate(root_model_matrix, angle_y, glm::vec3(0.0f, 1.0f, 0.0f));                           // Multiply model matrix by the rotation matrix around Y axis by angle_y degrees
    root_model_matrix = glm::rotate(root_model_matrix, angle_x, glm::vec3(1.0f, 0.0f, 0.0f));                           // Multiply model matrix by the rotation matrix around X axis by angle_x degrees
    glm::mat4 V = glm::lookAt(glm::vec3(0.0f, 6.0f, -15.0f), glm::vec3(0.0f, 2.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // Compute view matrix
    glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 50.0f);                                             // Compute projection matrix

    glm::mat4 water_model_matrix = glm::scale(
        glm::translate(
            root_model_matrix,
            glm::vec3(0, 1.f, 0)),
        glm::vec3(32, 1, 16));
    drawWater(Colored, P, V, water_model_matrix);
    for (Mesh *m : meshes)
    {
        if (m->name == "kolo")
        {
            glm::mat4 wheel_model_matrix = root_model_matrix;
            wheel_model_matrix = rotate_around(wheel_model_matrix, glm::vec3(-5, 0, 0), wheel_angle, glm::vec3(0, 0, 1));
            m->drawTexturedShaded(LambertTextured, P, V, wheel_model_matrix);
        }
        else
            m->drawTexturedShaded(LambertTextured, P, V, root_model_matrix);
        // m->draw(Colored, P, V, root_model_matrix);
    }

    glfwSwapBuffers(window); // Copy back buffer to the front buffer
}

int main(void)
{
    meshes = {};
    GLFWwindow *window; // Pointer to object that represents the application window

    glfwSetErrorCallback(error_callback); // Register error processing callback procedure

    if (!glfwInit())
    { // Initialize GLFW library
        fprintf(stderr, "Can't initialize GLFW.\n");
        exit(EXIT_FAILURE);
    }

    window = glfwCreateWindow(1280, 720, "OpenGL", NULL, NULL); // Create a window 500pxx500px titled "OpenGL" and an OpenGL context associated with it.

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

    // Main application loop
    float angle_x = 0; // declare variable for storing current rotation angle
    float angle_y = 0; // declare variable for storing current rotation angle
    float wheel_angle = 0;
    float time = 0;
    float deltaTime = 0;
    glfwSetTime(0);                        // clear internal timer
    while (!glfwWindowShouldClose(window)) // As long as the window shouldnt be closed yet...
    {
        deltaTime = glfwGetTime();
        angle_x += speed_x * deltaTime; // Compute an angle by which the object was rotated during the previous frame
        if (angle_x > TAU)
            angle_x -= TAU;
        angle_y += speed_y * deltaTime; // Compute an angle by which the object was rotated during the previous frame
        if (angle_y > TAU)
            angle_y -= TAU;
        wheel_angle += wheel_speed * deltaTime;
        if (wheel_angle > TAU)
            wheel_angle -= TAU;
        time += deltaTime;
        if (time > MAX_TIME)
            time -= MAX_TIME;
        glfwSetTime(0);                                         // clear internal timer
        drawScene(window, angle_x, angle_y, wheel_angle, time); // Execute drawing procedure
        glfwPollEvents();                                       // Process callback procedures corresponding to the events that took place up to now
    }
    freeOpenGLProgram(window);

    glfwDestroyWindow(window); // Delete OpenGL context and the window.
    glfwTerminate();           // Free GLFW resources
    exit(EXIT_SUCCESS);
}
