#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanContext.h"
#include "CentredCamera.h"
#include "FreeCamera.h"
#include "Timer.h"

class App {
private:
    GLFWwindow* window;
    Timer timer;
    CentredCamera centredCamera;
    FreeCamera freeCamera;
    std::unique_ptr<VulkanContext> vkContext;

    ApplicationOptions options;
public:
    void run();
    App(int width, int height, const char* pApplicationName, const char* pEngineName);
    ~App();
private:
    void initWindow(int width, int height);
};
