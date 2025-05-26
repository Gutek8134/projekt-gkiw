#define GLM_FORCE_RADIANS

#include <random>
#include <glm/gtc/matrix_transform.hpp>
#include "particle_system.hpp"
#include "constants.hpp"

std::vector<glm::vec3> generate_random_positions(int n, glm::vec4 origin, glm::vec3 deviation);
std::vector<glm::vec3> generate_random_velocities(int n, glm::vec4 up, glm::vec4 right, float base_speed, float speed_deviation, float max_angle);
std::vector<float> generate_random_lifetimes(int n, float base, float deviation);

ParticleSystem::ParticleSystem(glm::vec4 origin, glm::vec3 position_deviation, float spawn_rate, glm::vec4 direction, float max_angle, float initial_speed, float initial_speed_deviation, float drag, float lifetime, float lifetime_deviation, Mesh *particle_model, ShaderProgram *shader)
{
    this->origin = origin;
    this->position_deviation = position_deviation;
    this->spawn_rate = spawn_rate;
    this->direction = direction;
    this->max_angle = max_angle;
    this->initial_speed = initial_speed;
    this->initial_speed_deviation = initial_speed_deviation;
    this->drag = drag;
    this->lifetime = lifetime;
    this->lifetime_deviation = lifetime_deviation;
    this->particle = particle_model;
    this->shader = shader;
    particle_positions = {};
    particle_velocity = {};
    particle_lifetime_remaining = {};
}

void ParticleSystem::draw(float deltaTime, glm::mat4 P, glm::mat4 V, glm::mat4 root_object)
{
    static glm::vec4 right = glm::normalize(glm::vec4(glm::cross(glm::vec3(this->direction), glm::vec3(this->direction) + glm::vec3(1, 0, 1)), 1));
    // Erase particles that exceeded their lifetime
    for (int i = 0; i < particle_lifetime_remaining.size();)
    {
        if ((particle_lifetime_remaining.at(i) -= deltaTime) < 0)
        {
            particle_positions.erase(particle_positions.begin() + i);
            particle_velocity.erase(particle_velocity.begin() + i);
            particle_lifetime_remaining.erase(particle_lifetime_remaining.begin() + i);
        }
        else
            ++i;
    }

    for (int i = 0; i < particle_positions.size(); ++i)
    {
        // Move according to particle's velocity
        particle_positions.at(i) += particle_velocity.at(i) * deltaTime;
        // Decrease velocity
        particle_velocity.at(i) -= drag * particle_velocity.at(i) * deltaTime;

        particle->draw(shader, P, V, glm::translate(glm::mat4(1), particle_positions.at(i)));
    }

    int to_spawn = spawn_rate * deltaTime;

    // Create new particles
    auto new_positions = generate_random_positions(to_spawn, root_object * this->origin, this->position_deviation);
    auto new_velocities = generate_random_velocities(to_spawn, this->direction, right, this->initial_speed, this->initial_speed_deviation, this->max_angle);
    auto new_lifetimes = generate_random_lifetimes(to_spawn, this->lifetime, this->lifetime_deviation);

    particle_positions.insert(particle_positions.end(), new_positions.begin(), new_positions.end());
    particle_velocity.insert(particle_velocity.end(), new_velocities.begin(), new_velocities.end());
    particle_lifetime_remaining.insert(particle_lifetime_remaining.end(), new_lifetimes.begin(), new_lifetimes.end());
}

std::vector<glm::vec3> generate_random_positions(int n, glm::vec4 origin, glm::vec3 deviation)
{
    static std::random_device rng{};
    static std::mt19937 generator{rng()};
    static std::normal_distribution<float>
        position_distribution_x(-deviation.x, deviation.x),
        position_distribution_y(-deviation.y, deviation.y),
        position_distribution_z(-deviation.z, deviation.z);

    std::vector<glm::vec3> positions;
    positions.reserve(n);

    for (int i = 0; i < n; ++i)
    {
        positions.push_back(origin + glm::vec4(
                                         position_distribution_x(generator),
                                         position_distribution_y(generator),
                                         position_distribution_z(generator),
                                         0));
    }

    return positions;
};

std::vector<glm::vec3> generate_random_velocities(int n, glm::vec4 up, glm::vec4 right, float base_speed, float speed_deviation, float max_angle)
{
    static std::random_device rng{};
    static std::mt19937 generator{rng()};
    static std::normal_distribution<float> speed_distribution(base_speed, speed_deviation);
    // X angle - how far from up
    // Y angle - uniform spread on XZ plane
    static std::uniform_real_distribution<float> x_angle_distribution(0.f, max_angle), y_angle_distribution(0, TAU);

    std::vector<glm::vec3> velocities;
    velocities.reserve(n);

    for (int i = 0; i < n; ++i)
    {
        float x_angle = x_angle_distribution(generator),
              y_angle = y_angle_distribution(generator),
              speed = speed_distribution(generator);

        glm::mat4 velocity_matrix = glm::mat4(1.f);
        velocity_matrix = glm::rotate(velocity_matrix, x_angle, glm::vec3(right));
        velocity_matrix = glm::rotate(velocity_matrix, y_angle, glm::vec3(up));
        glm::vec4 velocity = (up * velocity_matrix) * speed;
        velocities.push_back(velocity);
    }

    return velocities;
}
std::vector<float> generate_random_lifetimes(int n, float base, float deviation)
{
    static std::random_device rng{};
    static std::mt19937 generator{rng()};
    static std::uniform_real_distribution<float> distribution(base - deviation, base + deviation);

    std::vector<float> lifetimes;
    lifetimes.reserve(n);

    for (int i = 0; i < n; i++)
    {
        lifetimes.push_back(distribution(generator));
    }

    return lifetimes;
}