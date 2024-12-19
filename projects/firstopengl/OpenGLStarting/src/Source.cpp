#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<stb_image.h>
#include <Shader.h>
#include <Camera.h>
#include <Model.h>
#include <fstream>

#include <iostream>

// Global variables
glm::vec3 target(0.0f, 0.0f, 0.0f);
float yaw = 0.0f;
float pitch = 0.0f;
bool isRightClickHeld = false;
bool isLeftClickHeld = false; // New boolean for left click hold
glm::vec3 cameraPos;
float distance_ = 0.3f;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// function prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void updateCamera(glm::vec3& cameraPos, glm::mat4& viewMatrix);


int main()
{
    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Orbit + Pan Camera", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Set our callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Start with the cursor visible
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    // load OpenGL functions
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // flip textures
    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    Shader ourShader("../shaders/model_loading.vs", "../shaders/model_loading.fs");
    Model ourModel("../resources/backpack/backpack.obj");

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        processInput(window);

        glm::mat4 view;
        updateCamera(cameraPos, view);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ourShader.use();
        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.1f, 0.1f, 0.1f));
        ourShader.setMat4("model", model);
        ourModel.Draw(ourShader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

// Normal input handling for ESC
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// Handle window resize
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

// This function updates the camera position based on yaw, pitch, distance_
void updateCamera(glm::vec3& cameraPos, glm::mat4& viewMatrix)
{
    cameraPos.x = target.x + distance_ * cos(pitch) * sin(yaw);
    cameraPos.y = target.y + distance_ * sin(pitch);
    cameraPos.z = target.z + distance_ * cos(pitch) * cos(yaw);

    viewMatrix = glm::lookAt(cameraPos, target, glm::vec3(0.0f, 1.0f, 0.0f));
}

// This callback handles mouse movement for both orbit (right-click) and pan (left-click)
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    static double lastX = xpos, lastY = ypos;

    double deltaX = xpos - lastX;
    double deltaY = ypos - lastY;

    // If right click is held, orbit the camera
    if (isRightClickHeld) {
        float sensitivity = 0.005f;
        yaw += (float)deltaX * sensitivity;
        pitch -= (float)deltaY * sensitivity;

        float limit = glm::radians(89.0f);
        if (pitch > limit) pitch = limit;
        if (pitch < -limit) pitch = -limit;
    }
    // If left click is held, pan the camera based on mouse movement
    else if (isLeftClickHeld) 
    {

        float panSpeed = 0.001f * distance_; // Adjust for sensitivity
        // Calculate directions
        glm::vec3 forward = glm::vec3(sin(yaw), 0, cos(yaw));
        glm::vec3 right = glm::normalize(glm::cross(forward, glm::vec3(0, 1, 0)));
        glm::vec3 up = glm::vec3(0, 1, 0);

        // deltaX < 0 => move target left, deltaX > 0 => move target right
        target += (float)(-deltaX * panSpeed) * right;

        // deltaY < 0 => move target forward, deltaY > 0 => move target backward
        // If you prefer "up/down" pan instead of forward/back, replace forward with up:
        target += (float)(deltaY * panSpeed) * up;
    }

    lastX = xpos;
    lastY = ypos;
}

// Handle mouse button presses: right for orbit, left for pan
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        isRightClickHeld = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        isRightClickHeld = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        isLeftClickHeld = true;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        isLeftClickHeld = false;
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
}

// Handle scroll for zoom
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    distance_ -= (float)yoffset * 0.5f;
    if (distance_ < 0.1f) distance_ = 0.1f;
    if (distance_ > 100.0f) distance_ = 100.0f;
}
