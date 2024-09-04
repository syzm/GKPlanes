#include <imgui/imgui.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_impl_glfw.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <misc/shader_m.h>
#include <misc/camera.h>
#include <misc/model.h>

#include <iostream>
#include "plane.h"
#include "skybox.h"

enum CameraMode {
    FREE_CAMERA,
    BEHIND_PLANE_CAMERA,
    SCENE_CAMERA,
    STATIC_TRACKING_CAMERA
};

enum ShadingMode {
    FLAT_SHADING,
    PHONG_SHADING,
    GOURAUD_SHADING
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void UpdateCameraPosition(CameraMode mode, Plane& plane);
void SetupImGui(GLFWwindow* window);
void RenderImGui(Skybox& skybox, std::vector<std::string>& dayFaces, std::vector<std::string>& nightFaces);
void InitializeBezierSurface(GLuint& bezierVAO, GLuint& bezierVBO);
void RenderBezierSurface(GLuint bezierVAO, GLuint bezierVBO, Shader& bezierShader, float currentFrame, Camera& camera);

bool cursorEnabled = false;
enum SkyboxType { DAY, NIGHT };
SkyboxType currentSkybox = DAY;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
CameraMode currentCameraMode = FREE_CAMERA;
ShadingMode currentShadingMode = PHONG_SHADING;
Camera camera(glm::vec3(0.0f, 11.0f, 8.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

const glm::vec3 BEHIND_PLANE_OFFSET = glm::vec3(0.0f, 2.0f, -5.0f);
const glm::vec3 SCENE_CAMERA_POSITION = glm::vec3(0.0f, 25.0f, 25.0f);
const glm::vec3 STATIC_TRACKING_POSITION = glm::vec3(0.0f, 15.0f, 15.0f);
glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));;
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 0.9f);;

// fog
glm::vec3 fogColor = glm::vec3(0.5f, 0.5f, 0.5f);  // Default fog color
float fogIntensity = 0.0f;

// street lights
glm::vec3 spotLight1Pos = glm::vec3(6.66f, 3.72f, 1.67f);
glm::vec3 spotLight2Pos = glm::vec3(-7.00f, 3.81f, 4.46f);
glm::vec3 spotLightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
float spotLightCutOff = glm::cos(glm::radians(12.5f));
float spotLightOuterCutOff = glm::cos(glm::radians(17.5f));
glm::vec3 spotLightColor = glm::vec3(1.0f, 0.95f, 0.8f);

// front plane light
glm::vec3 planeSpotLightPos;
glm::vec3 planeSpotLightDir;
float planeSpotLightCutOff = glm::cos(glm::radians(15.0f));
float planeSpotLightOuterCutOff = glm::cos(glm::radians(20.0f));
glm::vec3 planeSpotLightColor = glm::vec3(0.1f, 1.0f, 0.1f);

// under plane light
glm::vec3 underPlaneSpotLightPos;
glm::vec3 underPlaneSpotLightDir;
float underPlaneSpotLightCutOff = glm::cos(glm::radians(20.0f));
float underPlaneSpotLightOuterCutOff = glm::cos(glm::radians(25.0f));
glm::vec3 underPlaneSpotLightColor = glm::vec3(1.0f, 0.2f, 0.2f);
float underPlaneSpotLightPitch = 0.0f;
float underPlaneSpotLightRoll = 0.0f;

bool animateControlPoints = false;
bool showBezierSurface = false;
float animationSpeed = 0.5f;
glm::vec3 controlPoints[16] = {
    {0.0f, 1.0f, 0.0f}, {1.0f / 3.0f, 1.0f, 0.0f}, {2.0f / 3.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.0f},
    {0.0f, 2.0f / 3.0f, 0.0f}, {1.0f / 3.0f, 2.0f / 3.0f, 0.0f}, {2.0f / 3.0f, 2.0f / 3.0f, 0.0f}, {1.0f, 2.0f / 3.0f, 0.0f},
    {0.0f, 1.0f / 3.0f, 0.0f}, {1.0f / 3.0f, 1.0f / 3.0f, 0.0f}, {2.0f / 3.0f, 1.0f / 3.0f, 0.0f}, {1.0f, 1.0f / 3.0f, 0.0f},
    {0.0f, 0.0f, 0.0f}, {1.0f / 3.0f, 0.0f, 0.0f}, {2.0f / 3.0f, 0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}
};
glm::vec3 bezierSurfaceColor = glm::vec3(0.2f, 0.2f, 0.2f);


std::vector<std::string> dayFaces = {
    "resources/textures/skybox/day/right.bmp",
    "resources/textures/skybox/day/left.bmp",
    "resources/textures/skybox/day/top.bmp",
    "resources/textures/skybox/day/bottom.bmp",
    "resources/textures/skybox/day/front.bmp",
    "resources/textures/skybox/day/back.bmp"
};

std::vector<std::string> nightFaces = {
    "resources/textures/skybox/night/right.png",
    "resources/textures/skybox/night/left.png",
    "resources/textures/skybox/night/top.png",
    "resources/textures/skybox/night/bottom.png",
    "resources/textures/skybox/night/front.png",
    "resources/textures/skybox/night/back.png"
};

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    SetupImGui(window);

    GLuint bezierVAO, bezierVBO;
    InitializeBezierSurface(bezierVAO, bezierVBO);

    // Load shaders and models
    Shader phongShader("src/shaders/phong.vs", "src/shaders/phong.fs");
    Shader gouraudShader("src/shaders/gouraud.vs", "src/shaders/gouraud.fs");
    Shader flatShader("src/shaders/flat.vs", "src/shaders/flat.fs");
    Shader bezierShader("src/shaders/bezier.vs", "src/shaders/bezier.fs", "src/shaders/bezier.tcs", "src/shaders/bezier.tes");
    Shader skyboxShader("src/shaders/skybox.vs", "src/shaders/skybox.fs");
    Skybox skybox(dayFaces);
    Model sceneModel("resources/objects/winter/source/scene/winterScene.obj");
    Model planeModel("resources/objects/plane/source/Jet/Jet.obj");
    Plane plane(6.0f, 1.0f, planeModel);

    // render loop
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process input
        processInput(window);

        // Update the plane's position
        plane.Update(currentFrame);
        glm::vec3 forwardDir = plane.GetDirection();
        glm::vec3 upDir = plane.GetUpDirection();
        glm::vec3 planePosition = plane.GetPosition();
        glm::vec3 rightDir = glm::normalize(glm::cross(forwardDir, upDir));

        planeSpotLightPos = planePosition + forwardDir * 0.5f;
        planeSpotLightDir = forwardDir;
        // Pitch
        glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(underPlaneSpotLightPitch), rightDir);
        // Roll
        rotation = glm::rotate(rotation, glm::radians(underPlaneSpotLightRoll), forwardDir);
        underPlaneSpotLightDir = glm::vec3(rotation * glm::vec4(-upDir, 0.0f));
        underPlaneSpotLightPos = planePosition + underPlaneSpotLightDir * 0.1f;

        UpdateCameraPosition(currentCameraMode, plane);

        if (fogIntensity > 0.0f) {
            glClearColor(fogColor.r, fogColor.g, fogColor.b, 1.0f);
        }
        else {
            glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Start ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if (showBezierSurface) {
            RenderBezierSurface(bezierVAO, bezierVBO, bezierShader, currentFrame, camera);
        }

        // Set shaders and matrices
        Shader* activeShader = nullptr;
        switch (currentShadingMode) {
        case PHONG_SHADING:
            activeShader = &phongShader;
            break;
        case GOURAUD_SHADING:
            activeShader = &gouraudShader;
            break;
        case FLAT_SHADING:
            activeShader = &flatShader;
            break;
        }        
        activeShader->use();

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        activeShader->setMat4("projection", projection);
        activeShader->setMat4("view", view);
        activeShader->setVec3("lightDirection", lightDir);
        activeShader->setVec3("lightColor", lightColor);
        activeShader->setVec3("viewPos", camera.Position);
        activeShader->setVec3("fogColor", fogColor);
        activeShader->setFloat("fogIntensity", fogIntensity);
        activeShader->setVec3("spotLight1Pos", spotLight1Pos);
        activeShader->setVec3("spotLight1Dir", spotLightDir);
        activeShader->setVec3("spotLight2Pos", spotLight2Pos);
        activeShader->setVec3("spotLight2Dir", spotLightDir);
        activeShader->setVec3("spotLightColor", spotLightColor);
        activeShader->setFloat("spotLightCutOff", spotLightCutOff);
        activeShader->setFloat("spotLightOuterCutOff", spotLightOuterCutOff);
        activeShader->setVec3("planeSpotLightPos", planeSpotLightPos);
        activeShader->setVec3("planeSpotLightDir", planeSpotLightDir);
        activeShader->setVec3("planeSpotLightColor", planeSpotLightColor);
        activeShader->setFloat("planeSpotLightCutOff", planeSpotLightCutOff);
        activeShader->setFloat("planeSpotLightOuterCutOff", planeSpotLightOuterCutOff);
        activeShader->setVec3("underPlaneSpotLightPos", underPlaneSpotLightPos);
        activeShader->setVec3("underPlaneSpotLightDir", underPlaneSpotLightDir);
        activeShader->setVec3("underPlaneSpotLightColor", underPlaneSpotLightColor);
        activeShader->setFloat("underPlaneSpotLightCutOff", underPlaneSpotLightCutOff);
        activeShader->setFloat("underPlaneSpotLightOuterCutOff", underPlaneSpotLightOuterCutOff);
        activeShader->setFloat("constant", 1.0f);
        activeShader->setFloat("linear", 0.09f);
        activeShader->setFloat("quadratic", 0.032f);

        glm::mat4 model = glm::mat4(1.0f);
        activeShader->setMat4("model", model);
        sceneModel.Draw(*activeShader);
        
        plane.Draw(*activeShader);
        if (fogIntensity == 0.0f) {
            skybox.Draw(skyboxShader, camera.GetViewMatrix(), projection);
        }

        RenderImGui(skybox, dayFaces, nightFaces);
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up ImGui
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    static bool pKeyPressed = false; // Debounce flag

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Toggle cursor control with debouncing
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)
    {
        if (!pKeyPressed) // Only toggle if the key was not previously pressed
        {
            cursorEnabled = !cursorEnabled;
            if (cursorEnabled)
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            else
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

            pKeyPressed = true;
        }
    }
    else
    {
        pKeyPressed = false; // Reset debounce flag when the key is released
    }

    // Only process camera movement when the cursor is disabled and in FREE_CAMERA mode
    if (!cursorEnabled && currentCameraMode == FREE_CAMERA)
    {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.ProcessKeyboard(RIGHT, deltaTime);
    }
}


// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    // Skip processing if the cursor is enabled (ImGui interaction mode)
    if (cursorEnabled) return;

    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

void UpdateCameraPosition(CameraMode mode, Plane& plane) {
    switch (mode) {
    case BEHIND_PLANE_CAMERA: {
        glm::vec3 planePosition = plane.GetPosition();
        glm::vec3 planeDirection = plane.GetDirection();
        glm::vec3 cameraPosition = planePosition - planeDirection * glm::length(BEHIND_PLANE_OFFSET.z) + glm::vec3(0.0f, BEHIND_PLANE_OFFSET.y, 0.0f);
        camera.Position = cameraPosition;
        camera.Front = glm::normalize(planePosition - cameraPosition);
        break;
    }
    case SCENE_CAMERA: {
        camera.Position = SCENE_CAMERA_POSITION;
        camera.Front = glm::normalize(-camera.Position);
        camera.Right = glm::normalize(glm::cross(camera.Front, camera.WorldUp));
        camera.Up = glm::normalize(glm::cross(camera.Right, camera.Front));
        break;
    }
    case STATIC_TRACKING_CAMERA: {
        camera.Position = STATIC_TRACKING_POSITION;
        camera.Front = glm::normalize(plane.GetPosition() - camera.Position);
        break;
    }
    case FREE_CAMERA:
    default:
        break;
    }
}

void SetupImGui(GLFWwindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");
}

void RenderImGui(Skybox& skybox, std::vector<std::string>& dayFaces, std::vector<std::string>& nightFaces) {
    ImGui::Begin("Control Panel");
    ImGui::Text("Press 'P' to toggle cursor control.");

    const char* skyboxItems[] = { "Day", "Night" };
    int currentSkyboxIndex = static_cast<int>(currentSkybox);
    if (ImGui::Combo("Skybox", &currentSkyboxIndex, skyboxItems, IM_ARRAYSIZE(skyboxItems))) {
        if (currentSkyboxIndex == DAY && currentSkybox != DAY) {
            lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
            lightColor = glm::vec3(1.0f, 1.0f, 0.9f);
            currentSkybox = DAY;
            skybox.SwitchTextures(dayFaces);
        }
        else if (currentSkyboxIndex == NIGHT && currentSkybox != NIGHT) {
            lightDir = glm::normalize(glm::vec3(0.0f, -1.0f, 0.0f));
            lightColor = glm::vec3(0.5f, 0.5f, 0.7f);
            currentSkybox = NIGHT;
            skybox.SwitchTextures(nightFaces);
        }
    }

    const char* cameraModes[] = { "Free Camera", "Behind Plane Camera", "Scene Camera", "Static Tracking Camera" };
    int cameraModeIndex = static_cast<int>(currentCameraMode);
    if (ImGui::Combo("Camera Mode", &cameraModeIndex, cameraModes, IM_ARRAYSIZE(cameraModes))) {
        currentCameraMode = static_cast<CameraMode>(cameraModeIndex);
    }

    const char* shadingModes[] = { "Flat Shading", "Phong Shading", "Gouraud Shading" };
    int shadingModeIndex = static_cast<int>(currentShadingMode);
    if (ImGui::Combo("Shading Mode", &shadingModeIndex, shadingModes, IM_ARRAYSIZE(shadingModes))) {
        currentShadingMode = static_cast<ShadingMode>(shadingModeIndex);
    }

    ImGui::ColorEdit3("Fog Color", glm::value_ptr(fogColor));
    ImGui::SliderFloat("Fog Intensity", &fogIntensity, 0.0f, 1.0f);
    ImGui::Text("Camera Position: X: %.2f, Y: %.2f, Z: %.2f", camera.Position.x, camera.Position.y, camera.Position.z);
    ImGui::SliderFloat("Move Under Light Front-Back", &underPlaneSpotLightPitch, -90.0f, 90.0f, "%.1f degrees"); // Controls the pitch (up and down)
    ImGui::SliderFloat("Move Under Light Left-Right", &underPlaneSpotLightRoll, -180.0f, 180.0f, "%.1f degrees"); // Controls the roll (tilting left and right)
    ImGui::Checkbox("Show Bezier Surface", &showBezierSurface);
    if (showBezierSurface) {
        if (ImGui::Button(animateControlPoints ? "Stop Bezier Animation" : "Start Bezier Animation")) {
            animateControlPoints = !animateControlPoints;
        }
        if (animateControlPoints) {
            ImGui::SliderFloat("Bezier Animation Speed", &animationSpeed, 0.1f, 2.0f);
        }
        for (int i = 0; i < 16; ++i) {
            ImGui::PushID(i); // Push a unique ID for each slider to avoid conflicts
            ImGui::SliderFloat(("Control Point " + std::to_string(i + 1) + " Z").c_str(), &controlPoints[i].z, -1.0f, 1.0f);
            ImGui::PopID();
        }
    }
    ImGui::End();
}

void InitializeBezierSurface(GLuint& bezierVAO, GLuint& bezierVBO) {
    glGenVertexArrays(1, &bezierVAO);
    glGenBuffers(1, &bezierVBO);

    glBindVertexArray(bezierVAO);
    glBindBuffer(GL_ARRAY_BUFFER, bezierVBO);

    glBufferData(GL_ARRAY_BUFFER, sizeof(controlPoints), controlPoints, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void RenderBezierSurface(GLuint bezierVAO, GLuint bezierVBO, Shader& bezierShader, float currentFrame, Camera& camera) {
    if (animateControlPoints) {
        for (int i = 0; i < 16; ++i) {
            controlPoints[i].z = sin(currentFrame * animationSpeed + i) * 1.0f;
        }
    }
    glBindBuffer(GL_ARRAY_BUFFER, bezierVBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(controlPoints), controlPoints);

    bezierShader.use();
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model_surface = glm::mat4(1.0f);
    model_surface = glm::translate(model_surface, glm::vec3(-9, 6, 2));
    model_surface = glm::scale(model_surface, glm::vec3(3, 4, 2));

    bezierShader.setMat4("model", model_surface);
    bezierShader.setMat4("view", view);
    bezierShader.setMat4("projection", projection);
    bezierShader.setVec3("objectColor", bezierSurfaceColor);
    bezierShader.setVec3("lightDirection", lightDir);
    bezierShader.setVec3("lightColor", lightColor);
    bezierShader.setVec3("fogColor", fogColor);
    bezierShader.setFloat("fogIntensity", fogIntensity);
    bezierShader.setVec3("spotLight1Pos", spotLight1Pos);
    bezierShader.setVec3("spotLight1Dir", spotLightDir);
    bezierShader.setVec3("spotLight2Pos", spotLight2Pos);
    bezierShader.setVec3("spotLight2Dir", spotLightDir);
    bezierShader.setVec3("spotLightColor", spotLightColor);
    bezierShader.setFloat("spotLightCutOff", spotLightCutOff);
    bezierShader.setFloat("spotLightOuterCutOff", spotLightOuterCutOff);
    bezierShader.setVec3("planeSpotLightPos", planeSpotLightPos);
    bezierShader.setVec3("planeSpotLightDir", planeSpotLightDir);
    bezierShader.setVec3("planeSpotLightColor", planeSpotLightColor);
    bezierShader.setFloat("planeSpotLightCutOff", planeSpotLightCutOff);
    bezierShader.setFloat("planeSpotLightOuterCutOff", planeSpotLightOuterCutOff);
    bezierShader.setVec3("underPlaneSpotLightPos", underPlaneSpotLightPos);
    bezierShader.setVec3("underPlaneSpotLightDir", underPlaneSpotLightDir);
    bezierShader.setVec3("underPlaneSpotLightColor", underPlaneSpotLightColor);
    bezierShader.setFloat("underPlaneSpotLightCutOff", underPlaneSpotLightCutOff);
    bezierShader.setFloat("underPlaneSpotLightOuterCutOff", underPlaneSpotLightOuterCutOff);
    bezierShader.setFloat("constant", 1.0f);
    bezierShader.setFloat("linear", 0.09f);
    bezierShader.setFloat("quadratic", 0.032f);

    glPatchParameteri(GL_PATCH_VERTICES, 16);

    glBindVertexArray(bezierVAO);
    glDrawArrays(GL_PATCHES, 0, 16);
    glBindVertexArray(0);
}