#pragma once
#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <assimp/scene.h>
#include "shaderprogram.h"

class Mesh
{
public:
    std::vector<glm::vec4> vertex_positons = {};
    std::vector<glm::vec4> vertex_normals = {};
    std::vector<glm::vec2> texture_coordinates = {};
    std::vector<glm::ivec3> faces = {};
    std::string name;

    GLuint diffuse_texture;
    GLuint roughness_texture;

    Mesh(aiMesh *, const aiScene *);
    Mesh() = default;
    void draw(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M);
    void drawTextured(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M);
    void drawTexturedShaded(ShaderProgram *sp, glm::mat4 P, glm::mat4 V, glm::mat4 M);
    void initialize_draw_vertices();
    void initialize_draw_texture_coordinates();

    ~Mesh();

    std::vector<glm::vec4> draw_normals = {};

private:
    std::vector<glm::vec4> draw_vertices = {};
    std::vector<glm::vec2> draw_texture_coordinates = {};

    bool has_texture_coordinates = false;
};

GLuint readTexture(const char *filename);