#pragma once

#include <vector>
#include <string>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <misc/shader_m.h>

class Skybox {
public:
    Skybox(const std::vector<std::string>& faces);
    ~Skybox();

    void Draw(const Shader& shader, const glm::mat4& view, const glm::mat4& projection);

    void SwitchTextures(const std::vector<std::string>& faces);

private:
    unsigned int loadCubemap(const std::vector<std::string>& faces);
    unsigned int cubemapTexture;
    unsigned int skyboxVAO, skyboxVBO;

    void initSkybox();
};