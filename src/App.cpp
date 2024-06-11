#include "App.h"


void CursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

App::App(int width, int height, const char* pApplicationName, const char* pEngineName) {
    initWindow(width,height);
    vkContext = std::make_unique<VulkanContext>(window, pApplicationName, pEngineName);
}
App::~App() {
    vkContext.release();
    glfwDestroyWindow(window);
    glfwTerminate();
}
void App::initWindow(int width, int height) {
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);
    window = glfwCreateWindow(width,height, "DEMO 01", nullptr, nullptr);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetScrollCallback(window, (GLFWscrollfun)MouseScrollCallback);
    glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)MouseButtonCallback);
    glfwSetCursorPosCallback(window, (GLFWcursorposfun)CursorPosCallback);
    glfwGetCursorPos(window, &camera.xCursorPos, &camera.yCursorPos);
}
void App::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        vkContext->drawFrame(camera, timer.getDeltaTime());
    }
}
void CursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera->isMiddleButtonPressed) {
        double deltaX = xPos - camera->xCursorPos;
        double deltaY = yPos - camera->yCursorPos;
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        const glm::vec2 deltaXY(deltaX / width, deltaY / height);
        camera->rotateCamera(deltaXY);
    }
    camera->xCursorPos = xPos;
    camera->yCursorPos = yPos;
}
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    camera->cameraScroll(yoffset);
}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            camera->isMiddleButtonPressed = true;
        }
        else {
            camera->isMiddleButtonPressed = false;
        }
    }
}