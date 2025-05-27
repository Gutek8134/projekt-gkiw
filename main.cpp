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

#include <math.h>
#include <stdlib.h>
#include <iostream>
#include <stdio.h>
#include <vector>

#include "constants.hpp"
#include "shaderprogram.h"
#include "myCube.h"
#include "mesh.h"
#include "particle_system.hpp"

#define sky_color 0, 0.4f, 0.8f, 1
#define water_color 0, 0.3f, 1, 1
#define MAX_TIME 255
#define water_side_length 100

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

/// Generatez plane on XZ plane, centered at (0,0)
Mesh *generate_plane(unsigned int n = 2, float min = -1, float max = 1)
{
    assert(n > 1);
    Mesh *m = new Mesh();

    m->vertex_positons.reserve(n * n);
    m->vertex_normals.reserve(n * n);
    m->faces.reserve((n - 1) * (n - 1));
    m->has_texture_coordinates = true;
    m->diffuse_texture = readTexture("water.png");
    m->texture_coordinates = std::vector<glm::vec2>(n * n, glm::vec2(0.5f));

    for (unsigned int x = 0; x < n; ++x)
    {
        if (x != (n - 1))
            for (unsigned int z = 0; z < n; ++z)
            {
                m->vertex_positons.push_back(glm::vec4(min + (float)x / (n - 1) * (max - min), 0, min + (float)z / (n - 1) * (max - min), 1));
                if (z != (n - 1))
                {
                    unsigned int index = x * n + z;
                    m->faces.push_back(glm::ivec3(index, index + 1, index + n));
                    m->faces.push_back(glm::ivec3(index + 1, index + n + 1, index + n));
                }
            }
        else
            for (unsigned int z = 0; z < n; ++z)
            {
                m->vertex_positons.push_back(glm::vec4(min + (float)x / (n - 1) * (max - min), 0, min + (float)z / (n - 1) * (max - min), 1));
            }
    }
    m->vertex_normals = std::vector<glm::vec4>(m->faces.size() * 3, glm::vec4(0, 1, 0, 0));
    m->name = "plane";
    m->initialize_draw_vertices();
    m->initialize_draw_texture_coordinates();
    return m;
}

Mesh *generate_uvsphere(int segments_x, int segments_y, float radius)
{
    assert(segments_x > 2);
    assert(segments_y > 2);
    int stack_count = segments_y - 1, sector_count = segments_x;

    Mesh *m = new Mesh();
    m->vertex_positons.reserve((segments_y - 2) * segments_x + 2);
    m->vertex_normals.reserve((segments_y - 2) * segments_x + 2);
    m->faces.reserve(2 * stack_count * segments_x);

    float sector_step = 2 * PI / sector_count;
    float stack_step = PI / stack_count;
    float sector_angle, stack_angle;

    // Create vertices
    float x, y, z, xy;
    for (int i = 0; i <= stack_count; ++i)
    {
        stack_angle = PI / 2 - i * stack_step; // starting from pi/2 to -pi/2
        xy = radius * cosf(stack_angle);       // r * cos(u)
        z = radius * sinf(stack_angle);        // r * sin(u)

        for (int j = 0; j <= sector_count; ++j)
        {
            sector_angle = j * sector_step; // starting from 0 to 2pi

            x = xy * cosf(sector_angle); // r * cos(u) * cos(v)
            y = xy * sinf(sector_angle); // r * cos(u) * sin(v)
            m->vertex_positons.push_back(glm::vec4(x, y, z, 1));
            m->vertex_normals.push_back(glm::normalize(glm::vec4(x, y, z, 0) / radius));
        }
    }

    // k1--k1+1
    // |  / |
    // | /  |
    // k2--k2+1
    int k1, k2;
    glm::vec4 normal;
    for (int i = 0; i < stack_count; ++i)
    {
        k1 = i * (sector_count + 1); // beginning of current stack
        k2 = k1 + sector_count + 1;  // beginning of next stack

        for (int j = 0; j < sector_count; ++j, ++k1, ++k2)
        {
            // 2 triangles per sector excluding first and last stacks
            // k1 => k2 => k1+1
            if (i != 0)
            {
                m->faces.push_back(glm::vec3(k1, k2, k1 + 1));
            }

            // k1+1 => k2 => k2+1
            if (i != (stack_count - 1))
            {
                m->faces.push_back(glm::vec3(k1 + 1, k2, k2 + 1));
            }
        }
    }

    m->name = "uvsphere";
    m->initialize_draw_vertices();
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

ShaderProgram *Colored, *Lambert, *LambertTextured, *Water, *Smoke;
Mesh *plane, *uv_sphere;
ParticleSystem *smoke;

// Initialization code procedure
void initOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, at the program start************
    Colored = new ShaderProgram("v_colored.glsl", "f_colored.glsl");
    Lambert = new ShaderProgram("v_lambert.glsl", "f_lambert.glsl");
    LambertTextured = new ShaderProgram("v_lamberttextured.glsl", "f_lamberttextured.glsl");
    Water = new ShaderProgram("v_water.glsl", "f_water.glsl");
    Smoke = new ShaderProgram("v_smoke.glsl", "f_smoke.glsl");
    plane = generate_plane(water_side_length, -32, 32);
    uv_sphere = generate_uvsphere(12, 6, 0.3);
    smoke = new ParticleSystem(
        glm::vec4(3.3f, 8, 0.f, 1),
        glm::vec3(0.1),
        100.f,
        glm::vec4(0, 1, 0, 0),
        PI / 6,
        1.2f,
        0.3f,
        0.05f,
        5.f,
        1.f,
        uv_sphere,
        Smoke);
    glClearColor(sky_color); // Set color buffer clear color
    glEnable(GL_DEPTH_TEST); // Turn on pixel depth test based on depth buffer
    glfwSetKeyCallback(window, key_callback);
    load_scene("statek.obj");
}

// Release resources allocated by the program
void freeOpenGLProgram(GLFWwindow *window)
{
    //************Place any code here that needs to be executed once, after the main loop ends************
    delete Colored, Lambert, LambertTextured, Water, Smoke;
    for (Mesh *m : meshes)
    {
        delete m;
    }
    meshes.clear();
    delete plane, uv_sphere, smoke;
}

void drawWater(ShaderProgram *shader, glm::mat4 P, glm::mat4 V, glm::mat4 M, float phase)
{

    std::vector<glm::vec4> colors = std::vector<glm::vec4>(plane->faces.size() * 3, glm::vec4(water_color)),
                           offsets = std::vector<glm::vec4>(plane->faces.size() * 3),
                           face_normals = std::vector<glm::vec4>(plane->faces.size()),
                           vertex_normals = std::vector<glm::vec4>(plane->faces.size() * 3);

    const float size = 10;
    int j = 0, face_index = 0;
    for (const auto &face : plane->faces)
    {
        for (int i = 0; i < 3; ++i)
        {
            auto index = face[i];
            int x = index % water_side_length, y = index / water_side_length;
            offsets[j] = glm::vec4(0, (sin((x + y) / size + phase)), 0, 0);
            ++j;
        }
        face_normals[face_index++] = glm::normalize(glm::vec4(glm::cross(
                                                                  glm::vec3((plane->vertex_positons[face[0]] + offsets[j - 3]) - (plane->vertex_positons[face[2]] + offsets[j - 1])),
                                                                  glm::vec3((plane->vertex_positons[face[1]] + offsets[j - 2]) - (plane->vertex_positons[face[2]] + offsets[j - 1]))),
                                                              0));
    }

    for (int i = 0; i < face_normals.size(); ++i)
        vertex_normals[3 * i] = vertex_normals[3 * i + 1] = vertex_normals[3 * i + 2] = face_normals[i];

    glEnableVertexAttribArray(shader->getAttributeLocation("colors"));
    glEnableVertexAttribArray(shader->getAttributeLocation("normals"));
    glEnableVertexAttribArray(shader->getAttributeLocation("offset"));
    glVertexAttribPointer(shader->getAttributeLocation("colors"), 4, GL_FLOAT, false, 0, colors.data());
    glVertexAttribPointer(shader->getAttributeLocation("normals"), 4, GL_FLOAT, false, 0, vertex_normals.data());
    glVertexAttribPointer(shader->getAttributeLocation("offset"), 4, GL_FLOAT, false, 0, offsets.data());
    plane->drawTextured(shader, P, V, M);
    glDisableVertexAttribArray(shader->getAttributeLocation("colors"));
    glDisableVertexAttribArray(shader->getAttributeLocation("normals"));
    glDisableVertexAttribArray(shader->getAttributeLocation("offset"));
}

// Drawing procedure
void drawScene(GLFWwindow *window, float angle_x, float angle_y, float wheel_angle, float time, float deltaTime)
{
    //************Place any code here that draws something inside the window******************l
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // Clear color and depth buffers

    glm::mat4 root_model_matrix = glm::mat4(1.0f);
    glm::vec3 camera_position = glm::vec4(0, 30, 0, 0),
              focus_point = glm::vec3(0, 1, 0),
              up = glm::vec3(0, 1, 0),
              right = glm::vec3(0, 0, 1),
              camera_to_focus = (camera_position - focus_point),
              direction = glm::normalize(camera_to_focus);
    glm::mat4 camera_model_matrix = glm::rotate(root_model_matrix, angle_x, glm::vec3(0.0f, 0.0f, 1.0f));
    camera_model_matrix = glm::rotate(camera_model_matrix, angle_y, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 V = glm::lookAt(glm::vec3(glm::vec4(camera_to_focus, 1) * camera_model_matrix) + focus_point, focus_point, up);
    glm::mat4 P = glm::perspective(glm::radians(50.0f), 1.0f, 1.0f, 100.0f);

    glm::mat4 water_model_matrix = glm::translate(
        root_model_matrix,
        glm::vec3(0, 0.25f, 0));

    static const float frequency = 0.5;
    float phase = frequency * time;
    root_model_matrix = glm::translate(root_model_matrix, glm::vec3(0, sin(water_side_length - phase) - 0.4, 0));
    drawWater(Water, P, V, water_model_matrix, phase);
    for (Mesh *m : meshes)
    {
        if (m->name == "kolo")
        {
            glm::mat4 wheel_model_matrix = root_model_matrix;
            wheel_model_matrix = rotate_around(wheel_model_matrix, glm::vec3(-4.7, 0, 0), wheel_angle, glm::vec3(0, 0, 1));
            m->drawTexturedShaded(LambertTextured, P, V, wheel_model_matrix);
        }
        else
            m->drawTexturedShaded(LambertTextured, P, V, root_model_matrix);
        // m->draw(Colored, P, V, root_model_matrix);
    }

    const glm::vec4 lightSource = smoke->get_origin();
    glUniform4f(smoke->shader->getUniformLocation("lightSource"), lightSource.x, lightSource.y - 0.1, lightSource.z, lightSource.w);
    smoke->draw(deltaTime, P, V, root_model_matrix);

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
    float angle_x = -PI / 6; // declare variable for storing current rotation angle
    float angle_y = 0;       // declare variable for storing current rotation angle
    float wheel_angle = 0;
    float time = 0;
    float deltaTime = 0;
    const float max_angle_x = PI / 2 - 0.2;
    glfwSetTime(0);                        // clear internal timer
    while (!glfwWindowShouldClose(window)) // As long as the window shouldnt be closed yet...
    {
        deltaTime = glfwGetTime();
        angle_x += speed_x * deltaTime; // Compute an angle by which the object was rotated during the previous frame
        angle_x = glm::clamp(angle_x, -max_angle_x, max_angle_x);
        angle_y = angle_y + speed_y * deltaTime; // Compute an angle by which the object was rotated during the previous frame
        if (angle_y > TAU)
            angle_y -= TAU;
        else if (angle_y < -TAU)
            angle_y += TAU;
        wheel_angle += wheel_speed * deltaTime;
        if (wheel_angle > TAU)
            wheel_angle -= TAU;
        time += deltaTime;
        if (time > MAX_TIME)
            time -= MAX_TIME;
        glfwSetTime(0);                                                    // clear internal timer
        drawScene(window, angle_x, angle_y, wheel_angle, time, deltaTime); // Execute drawing procedure
        glfwPollEvents();                                                  // Process callback procedures corresponding to the events that took place up to now
    }
    freeOpenGLProgram(window);

    glfwDestroyWindow(window); // Delete OpenGL context and the window.
    glfwTerminate();           // Free GLFW resources
    exit(EXIT_SUCCESS);
}
