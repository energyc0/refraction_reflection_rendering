#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "VulkanContext.h"
#include "Camera.h"
#include "Timer.h"

class App {
private:
    GLFWwindow* window;
    Timer timer;
    Camera camera;
    std::unique_ptr<VulkanContext> vkContext;
public:
    void run();
    App(int width, int height, const char* pApplicationName, const char* pEngineName);
    ~App();
private:
    void initWindow(int width, int height);
};
