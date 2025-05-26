#pragma once
#include <glm/glm.hpp>
#include <vector>

#include "mesh.h"
#include "shaderprogram.h"

class ParticleSystem
{
public:
    ParticleSystem(glm::vec4 origin, glm::vec3 position_deviation, float spawn_rate, glm::vec4 direction, float max_angle, float initial_speed, float initial_speed_deviation, float drag, float lifetime, float lifetime_deviation, Mesh *particle_model, ShaderProgram *shader);

    void draw(float deltaTime, glm::mat4 P, glm::mat4 V, glm::mat4 root_object = glm::mat4(1.f));
    ShaderProgram *shader;

private:
    glm::vec4 origin, direction;
    glm::vec3 position_deviation;
    float spawn_rate, max_angle, initial_speed, initial_speed_deviation, lifetime, lifetime_deviation, drag;
    std::vector<glm::vec3> particle_positions, particle_velocity;
    std::vector<float> particle_lifetime_remaining;
    Mesh *particle;
};