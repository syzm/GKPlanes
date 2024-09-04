#include "plane.h"
#include <cmath>

Plane::Plane(float radius, float speed, Model& model)
    : radius(radius), speed(speed), deltaAngle(0.01f), minHeight(8.0f), amplitude(2.0f), model(model) {
    position = glm::vec3(0.0f, minHeight, 0.0f);
}

void Plane::Update(float time) {
    UpdatePosition(time);
}

void Plane::UpdatePosition(float time) {
    float angle = time * speed;

    float planeY = minHeight + amplitude * (0.5f * (1.0f + sin(time * 0.5f)));
    float planeX = radius * cos(angle);
    float planeZ = radius * sin(angle);
    float nextPlaneX = radius * cos(angle + deltaAngle);
    float nextPlaneZ = radius * sin(angle + deltaAngle);

    glm::vec3 horizontalDirection = glm::normalize(glm::vec3(nextPlaneX - planeX, 0.0f, nextPlaneZ - planeZ));

    // Calculate the rate of change of the height to get pitch
    float heightChange = amplitude * 0.25f * cos(time * 0.5f);
    float pitch = atan2(heightChange, speed * radius);

    // Adjust direction to account for pitch
    direction = glm::normalize(glm::vec3(horizontalDirection.x, heightChange, horizontalDirection.z));

    glm::vec3 upVector = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::normalize(glm::cross(upVector, direction));
    up = glm::normalize(glm::cross(direction, right));

    glm::mat4 rotationMat = glm::mat4(
        glm::vec4(right, 0.0f),
        glm::vec4(up, 0.0f),
        glm::vec4(direction, 0.0f),
        glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
    );

    position = glm::vec3(planeX, planeY, planeZ);
    modelMatrix = glm::translate(glm::mat4(1.0f), position) * rotationMat;
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
}

void Plane::Draw(Shader& shader) {
    // Set the model matrix and draw the plane
    shader.setMat4("model", modelMatrix);
    model.Draw(shader);
}