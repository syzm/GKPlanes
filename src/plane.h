#ifndef PLANE_H
#define PLANE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <misc/shader_m.h>
#include <misc/model.h>

class Plane {
public:
    Plane(float radius, float speed, Model& model);
    void Update(float time);
    void Draw(Shader& shader);
    glm::vec3 GetPosition() const { return position; }
    glm::vec3 GetDirection() const { return direction; }
    glm::vec3 GetUpDirection() const { return up; }
private:
    float radius;         // Radius of the path
    float speed;          // Speed of the plane
    float deltaAngle;     // Small step to estimate direction
    float minHeight;      // Minimum height to maintain
    float amplitude;      // Amplitude of the y parabola

    glm::vec3 position;   // Current position of the plane
    glm::vec3 direction;  // Current direction vector of the plane
    glm::mat4 modelMatrix; // Model matrix for rendering
    glm::vec3 up;
    Model& model;         // Reference to the plane's model

    void UpdatePosition(float time); // Update the plane's position and direction
};

#endif