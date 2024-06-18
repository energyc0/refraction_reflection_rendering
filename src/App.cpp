#include "App.h"

extern bool isWindowResized;
bool isMiddleButtonPressed = false;
std::array<bool, 1024> keys;

void CursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void WindowResizeCallback(GLFWwindow* window, int width, int height);
void KeyboardCallback(GLFWwindow* window, int key, int scancode,int action, int mods);

App::App(int width, int height, const char* pApplicationName, const char* pEngineName) :
    timer(10.f),
    centredCamera(glm::vec3(1.f,0.f,0.f), glm::vec3(0.f),1.0f,1.0f),
    freeCamera(glm::vec3(0.f, 0.f, 400.f), glm::vec3(0.f), 1.f,1.f) {
    initWindow(width,height);
    vkContext = std::make_unique<VulkanContext>(window, pApplicationName, pEngineName, options);
    options.mode = static_cast<int>(DrawingMode::SOLID);
    options.modelSize = glm::vec3(1.0f);
    options.scrollSpeed = 50.f;
    options.currentCamera = &freeCamera;
    options.isInterfaceShown = true;
    options.isImageToSave = false;
    keys.fill(false);
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
    glfwSetWindowUserPointer(window, &options);
    glfwSetScrollCallback(window, (GLFWscrollfun)MouseScrollCallback);
    glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)MouseButtonCallback);
    glfwSetCursorPosCallback(window, (GLFWcursorposfun)CursorPosCallback);
    glfwSetKeyCallback(window, (GLFWkeyfun)KeyboardCallback);
    glfwSetWindowSizeCallback(window, (GLFWframebuffersizefun)WindowResizeCallback);
    double x, y;
    glfwGetCursorPos(window, &x, &y);
}
void App::run() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        timer.Tick();
        float deltaTime = timer.getDeltaTime();
        options.currentCamera->cameraProcess(deltaTime);
        vkContext->drawFrame(options, deltaTime);
    }
}
void CursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto options = reinterpret_cast<ApplicationOptions*>(glfwGetWindowUserPointer(window));
    int w, h;
    glfwGetWindowSize(window, &w, &h);
    options->currentCamera->rotateCamera(xPos, yPos, w, h);
}
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    //auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    //camera->cameraScroll(yoffset);
}
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_MIDDLE: 
        isMiddleButtonPressed = action == GLFW_PRESS ? true : false;
        break;
    default:
        break;
    }
    return;
}
void WindowResizeCallback(GLFWwindow* window, int width, int height) {
    isWindowResized = true;
}
void KeyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    auto options = reinterpret_cast<ApplicationOptions*>(glfwGetWindowUserPointer(window));
    if (action != GLFW_REPEAT) {
        keys[key] = action == GLFW_PRESS;
    }
    if(key == GLFW_KEY_ESCAPE){
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    else if(key == GLFW_KEY_F11 && action == GLFW_PRESS){
        options->isInterfaceShown = options->isInterfaceShown == false;
    }
    else if(key == GLFW_KEY_F9) {
        options->isImageToSave = action == GLFW_PRESS;
    }
}