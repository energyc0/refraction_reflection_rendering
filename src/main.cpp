#include <iostream>
#include <assert.h>
#include <filesystem>
#include <vector>
#include <array>
#include <vulkan/vulkan.h>
#include <fstream>
#include <optional>
#include <chrono>
#include <filesystem>
#include <istream>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#define IMGUI_IMPLEMENTATION
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
using std::vector;
using std::array;

using glm::vec3;
using glm::vec4;
using glm::mat4;

constexpr int WIDTH = 800;
constexpr int HEIGHT = 600;

const std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
std::string obj_filename = "models/Duck.obj";
std::string mtl_filename = "models";

struct swapchainImageProperty {
    VkFormat format;
    uint32_t width;
    uint32_t height;
};

struct PushConstantData {
    float sinus;
    VkBool32 isWireframeShown;
};

struct TransformMatricesData {
    mat4 model;
    mat4 view;
    mat4 perspective;
};

#ifdef NDEBUG
const bool isDebugging = false;
#else // NDEBUG
const bool isDebugging = true;
#endif

struct VertexData {
    vec3 pos;
    alignas(16)vec3 color;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        binding.stride = sizeof(VertexData);
        return binding;
    }
    static vector<VkVertexInputAttributeDescription> getAttributeDescription() {
        vector<VkVertexInputAttributeDescription>description(2);
        description[0].binding = 0;
        description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        description[0].location = 0;
        description[0].offset = offsetof(VertexData, pos);

        description[1].binding = 0;
        description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        description[1].location = 1;
        description[1].offset = offsetof(VertexData, color);
        return description;
    }
};

struct QueueFamilies {
    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> presentIndex;
    bool isComplete() {
        return graphicsIndex.has_value() && presentIndex.has_value();
    }
};

void VK_ASSERT(VkBool32 expr) {
    assert(expr == VK_SUCCESS);
}

VkBool32 debugMessengerCallback(VkDebugUtilsMessageTypeFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageTypes,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) {
    std::cerr << "\nVALIDATION LAYER: " << pCallbackData->pMessage;
    return VK_FALSE;
}

struct InstanceStructure {
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
};

class Timer {
    std::chrono::steady_clock::time_point lastTimePoint;
    std::chrono::steady_clock::time_point startTimePoint;
public:
    Timer();
    float getDeltaTime();
    float getProgramTime();
};

void CursorPosCallback(GLFWwindow* window, double xPos, double yPos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void MouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

class Camera {
    vec3 pos = vec3(350.5f, 350.5f, 350.5f);
public:
    double xCursorPos = 0.f;
    double yCursorPos = 0.f;
    bool isLeftButtonPressed = false;
    bool isRightButtonPressed = false;
    bool isMiddleButtonPressed = false;
    float scrollSpeed = 50.0f;
public:
    mat4 getCameraView();
    vec3 getPos();
    void cameraScroll(double yOffset);
    void rotateCamera(mat4& mat);
};

class App {
private:
    GLFWwindow* window;
    Timer timer;
    Camera camera;
    InstanceStructure instanceStruct;
    VkPhysicalDevice physicalDevice;
    VkDevice device;
    VkSwapchainKHR swapchain;
    QueueFamilies queueFamilies;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkCommandPool commandPool;
    vector<VkCommandBuffer> commandBuffers;
    uint32_t imageCount;
    swapchainImageProperty swapchainImageProperty;
    vector<VkImage> swapchainImages;
    vector<VkImageView> swapchainImageViews;
    VkPipeline graphicsPipeline;
    VkRenderPass renderPass;
    VkPipelineLayout pipelineLayout;
    vector<VkFramebuffer> framebuffers;
    vector<VkSemaphore> imageReadySemaphores;
    vector<VkSemaphore> readyToRenderSemaphores;
    vector<VkFence> drawFrameFences;
    uint32_t currentImage = 0;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkDescriptorSetLayout descriptorSetLayout;
    VkDescriptorPool descriptorPool;
    vector<VkDescriptorSet> descriptorSets;
    vector<VkBuffer> uniformBuffers;
    vector<VkDeviceMemory> uniformBuffersMemory;
    vector<void*> uniformBuffersMemoryPointers;
    mat4 modelMatrix = mat4(1.f);
    VkImage depthImage;
    VkImageView depthImageView;
    VkDeviceMemory depthImageMemory;
    VkFormat depthFormat;
    std::vector<VertexData> vertices;
    std::vector<uint32_t> indices;
    VkImage textureImage;
    VkImageView textureImageView;
    VkSampler textureSampler;
    array<float, 3> modelSize = { 1.f,1.f,1.f };
    bool isWireframeShown = false;
public:
    void run();
private:
    void init();
    void initWindow();
    void initVulkan();
    void mainLoop();
    void appShutdown();

    void createInstance();
    void pickPhysicalDevice();
    void createDevice();
    void createSwapchain();
    VkResult createDebugUtilsMessenger(VkDebugUtilsMessengerCreateInfoEXT* createInfo);
    void destroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT messenger);
    void createCommandPool();
    void createVertexBuffer();
    void createIndexBuffer();
    void createRenderPass();
    void createGraphicsPipelines();
    void createPipelineLayout();
    void drawFrame(float deltaTime);
    void createFrameBuffers();
    void createSyncObjects();
    void createDescriptorTools();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t i, float deltaTime);
    void createDepthResources();
    void recreateSwapchain();
    void cleanupSwapchain();
    void initImGui();

    bool isPhysicalDeviceSupported(VkPhysicalDevice physicalDevice);
    bool isValidationLayersSupported();
    std::vector<const char*> getExtensions();
    void createImageView(VkImageView* imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect);
    void createBuffer(VkBuffer* buffer, VkDeviceMemory* memory, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties);
    void copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t size);
    VkCommandBuffer beginSingleCommandBuffer();
    void endSingleCommandBuffer(VkCommandBuffer commandBuffer);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void loadModel(const char* obj_filename, const char* mtl_filename, VkImage* image, VkImageView* imageView, VkSampler* sampler);
    vector<char> readFile(const char* filename);
    VkShaderModule createShaderModule(const char* filename);
    void createTexture(VkImage* image, VkImageView* imageView, VkSampler* sampler);
    void recordCommandBuffer(uint32_t i, float deltaTime);
    VkFormat getSupportedFormat(vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    bool hasStencilComponent(VkFormat format);
    void createImage(VkImage* image, VkDeviceMemory* memory, uint32_t width, uint32_t height, VkImageTiling tiling, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkMemoryPropertyFlagBits property);
};

void App::initVulkan() {
    createInstance();
    createDevice();
    createSwapchain();
    createCommandPool();
    loadModel(obj_filename.c_str(), mtl_filename.c_str(), &textureImage, &textureImageView, &textureSampler);
    createVertexBuffer();
    createIndexBuffer();
    createUniformBuffers();
    createDescriptorTools();
    createDepthResources();
    createRenderPass();
    createFrameBuffers();
    createPipelineLayout();
    createGraphicsPipelines();
    createSyncObjects();
    initImGui();

}

void App::appShutdown() {
    cleanupSwapchain();
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    for (uint32_t i = 0; i < uniformBuffers.size(); i++) {
        vkUnmapMemory(device, uniformBuffersMemory[i]);
        vkDestroyBuffer(device, uniformBuffers[i], nullptr);
        vkFreeMemory(device, uniformBuffersMemory[i], nullptr);
    }
    for (uint32_t i = 0; i < imageReadySemaphores.size(); i++) {

        vkDestroySemaphore(device, imageReadySemaphores[i], nullptr);
        vkDestroySemaphore(device, readyToRenderSemaphores[i], nullptr);
        vkDestroyFence(device, drawFrameFences[i], nullptr);
    }
    vkDestroyBuffer(device, vertexBuffer, NULL);
    vkDestroyBuffer(device, indexBuffer, NULL);
    vkFreeMemory(device, vertexBufferMemory, NULL);
    vkFreeMemory(device, indexBufferMemory, NULL);
    vkDestroyCommandPool(device, commandPool, NULL);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(instanceStruct.instance, instanceStruct.surface, NULL);
    if (isDebugging) {
        destroyDebugUtilsMessenger(instanceStruct.debugMessenger);
    }
    vkDestroyInstance(instanceStruct.instance, NULL);
    glfwDestroyWindow(window);
    glfwTerminate();
}

void App::recreateSwapchain() {
    vkDeviceWaitIdle(device);
    cleanupSwapchain();
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwPollEvents();
        glfwGetWindowSize(window, &width, &height);
    }
    createSwapchain();
    createDepthResources();
    createRenderPass();
    createFrameBuffers();
    createPipelineLayout();
    createGraphicsPipelines();
    initImGui();
}

void App::cleanupSwapchain() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    for (uint32_t i = 0; i < framebuffers.size(); i++) {
        vkDestroyFramebuffer(device, framebuffers[i], nullptr);
        vkDestroyImageView(device, swapchainImageViews[i], NULL);
    }
    vkDestroyImageView(device, depthImageView, nullptr);
    vkDestroyImage(device, depthImage, nullptr);
    vkFreeMemory(device, depthImageMemory, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    vkDestroyRenderPass(device, renderPass, nullptr);
    vkDestroySwapchainKHR(device, swapchain, NULL);
}

void App::initImGui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        std::cerr << "FAILED TO INIT IMGUI + GLFW!";
        VK_ASSERT(VK_ERROR_INITIALIZATION_FAILED);
    }
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, instanceStruct.surface, &surfaceCapabilities);
    ImGui_ImplVulkan_InitInfo info{};
    info.RenderPass = renderPass;
    info.PhysicalDevice = physicalDevice;
    info.Device = device;
    info.ImageCount = imageCount;
    info.Instance = instanceStruct.instance;
    info.DescriptorPool = descriptorPool;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.MinImageCount = surfaceCapabilities.minImageCount;
    info.Queue = graphicsQueue;
    if (!ImGui_ImplVulkan_Init(&info)) {
        std::cerr << "FAILED TO INIT IMGUI + VULKAN!";
        VK_ASSERT(VK_ERROR_INITIALIZATION_FAILED);
    }
    //auto io = ImGui::GetIO();
}

void App::mainLoop() {
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();
        drawFrame(timer.getDeltaTime());
    }
    vkDeviceWaitIdle(device);
}

void App::createInstance() {
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    if (isDebugging) {
        assert(isValidationLayersSupported());
        createInfo.ppEnabledLayerNames = layers.data();
        createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
        VkDebugUtilsMessengerCreateInfoEXT createInfoM{};
        createInfoM.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfoM.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
        createInfoM.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfoM.pfnUserCallback = (PFN_vkDebugUtilsMessengerCallbackEXT)debugMessengerCallback;
        createInfo.pNext = &createInfoM;
    }
    else {
        createInfo.ppEnabledLayerNames = 0;
        createInfo.enabledLayerCount = 0;
    }
    auto extens = getExtensions();
    createInfo.ppEnabledExtensionNames = extens.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extens.size());
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.apiVersion = VK_API_VERSION_1_3;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pApplicationName = "DEMO 1";
    appInfo.pEngineName = "No Engine";
    createInfo.pApplicationInfo = &appInfo;
    VK_ASSERT(vkCreateInstance(&createInfo, NULL, &instanceStruct.instance));
    if (isDebugging) {
        createDebugUtilsMessenger((VkDebugUtilsMessengerCreateInfoEXT*)createInfo.pNext);
    }
    auto result = glfwCreateWindowSurface(instanceStruct.instance, window, NULL, &instanceStruct.surface);
    VK_ASSERT(result);
}

VkResult App::createDebugUtilsMessenger(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instanceStruct.instance, "vkCreateDebugUtilsMessengerEXT");
    if (func == nullptr) {
        VK_ASSERT(VK_ERROR_EXTENSION_NOT_PRESENT);
    }
    else {
        return func(instanceStruct.instance, createInfo, NULL, &instanceStruct.debugMessenger);
    }
    return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void App::destroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT messenger) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instanceStruct.instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func == nullptr) {
        VK_ASSERT(VK_ERROR_EXTENSION_NOT_PRESENT);
    }
    else {
        func(instanceStruct.instance, messenger, NULL);
    }
}

bool App::isValidationLayersSupported() {
    uint32_t count;
    vkEnumerateInstanceLayerProperties(&count, nullptr);
    std::vector<VkLayerProperties> l;
    l.resize(count);
    vkEnumerateInstanceLayerProperties(&count, l.data());

    for (auto& i : layers) {
        bool isFound = false;
        for (auto& layer : l) {
            if (std::strcmp(layer.layerName, i) == 0) {
                isFound = true;
                break;
            }
        }
        if (!isFound)
            return false;
    }

    return true;
}

std::vector<const char*> App::getExtensions() {
    uint32_t count;
    auto ext = glfwGetRequiredInstanceExtensions(&count);
    std::vector<const char*> result(ext, ext + count);
    if (isDebugging) {
        result.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }
    return result;
}

void App::drawFrame(float deltaTime) {
    vkWaitForFences(device, 1, &drawFrameFences[currentImage], VK_TRUE, UINT64_MAX);
    vkResetFences(device, 1, &drawFrameFences[currentImage]);
    uint32_t imageIndex;
    VkResult res = vkAcquireNextImageKHR(device, swapchain, UINT64_MAX, readyToRenderSemaphores[currentImage], NULL, &imageIndex);
    if (res != VK_SUCCESS) {
        std::cerr << "FAILED TO ACQUIRE NEXT IMAGE!\n";
    }
    recordCommandBuffer(currentImage,deltaTime);
    updateUniformBuffer(currentImage, deltaTime);
    VkPipelineStageFlags waitDstStageMask[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[currentImage];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &imageReadySemaphores[currentImage];
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &readyToRenderSemaphores[currentImage];
    submitInfo.pWaitDstStageMask = waitDstStageMask;
    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, drawFrameFences[currentImage])) {
        std::cerr << "FAILED TO SUBMIT QUEUE!\n";
    }
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.pImageIndices = &imageIndex;
    presentInfo.pSwapchains = &swapchain;
    presentInfo.swapchainCount = 1;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = &imageReadySemaphores[currentImage];
    res = vkQueuePresentKHR(presentQueue, &presentInfo);
    if (res == VK_SUBOPTIMAL_KHR || res == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapchain();
    }else if (res != VK_SUCCESS) {
        std::cerr << "FAILED TO PRESENT AN IMAGE!";
    }
    vkQueueWaitIdle(presentQueue);
    currentImage = (currentImage + 1) % imageCount;
}

void App::createFrameBuffers() {
    framebuffers.resize(imageCount);
    for (uint32_t i = 0; i < framebuffers.size(); i++) {
        array<VkImageView, 2> attachments{ swapchainImageViews[i], depthImageView };
        VkFramebufferCreateInfo createinfo{};
        createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        createinfo.width = swapchainImageProperty.width;
        createinfo.height = swapchainImageProperty.height;
        createinfo.renderPass = renderPass;
        createinfo.layers = 1;
        createinfo.attachmentCount = attachments.size();
        createinfo.pAttachments = attachments.data();
        VK_ASSERT(vkCreateFramebuffer(device, &createinfo, nullptr, &framebuffers[i]));
    }
}

void App::recordCommandBuffer(uint32_t i, float deltaTime) {
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    VkRenderPassBeginInfo renderPassBegin{};
    renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBegin.renderPass = renderPass;
    renderPassBegin.framebuffer = framebuffers[i];
    renderPassBegin.clearValueCount = 1;
    array<VkClearValue, 2> clearValues;
    clearValues[0].color = { 0.3f,0.3f,0.3f,1.0f };
    clearValues[1].depthStencil = { 1.0f,0 };
    renderPassBegin.pClearValues = clearValues.data();
    renderPassBegin.clearValueCount = clearValues.size();
    renderPassBegin.renderArea.offset = { 0,0 };
    renderPassBegin.renderArea.extent = { swapchainImageProperty.width, swapchainImageProperty.height };
    VkDeviceSize offsets[] = { 0 };
    VkViewport viewport{};
    viewport.x = viewport.y = 0;
    viewport.width = swapchainImageProperty.width;
    viewport.height = swapchainImageProperty.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    PushConstantData data;
    data.sinus = abs(sin(timer.getProgramTime()));
    data.isWireframeShown = isWireframeShown;
    VkCommandBuffer commandBuffer= commandBuffers[i];
    VK_ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo));
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &data);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[i], 0, 0);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, offsets);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdDrawIndexed(commandBuffer, indices.size(), 1, 0, 0, 0);
    bool isOpened = true;
    auto pos = camera.getPos();
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    auto& style = ImGui::GetStyle();
    ImGui::Begin("123", &isOpened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse  | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize/* | ImGuiWindowFlags_NoBackground*/);
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Text((std::string("FPS: ") + std::to_string(static_cast<int>(1000.f / deltaTime))).c_str());
    ImGui::Text((std::string("Total vertices: ") + std::to_string(vertices.size())).c_str());
    //ImGui::Text(std::string("Pitch: " + std::to_string(camera.pitch) + " Yaw: " + std::to_string(camera.yaw)).c_str());
    ImGui::Text((std::string("Camera position:\nX = ") + std::to_string(pos.x) + "\nY = " + std::to_string(pos.y) + "\nZ = " + std::to_string(pos.y)).c_str());
    ImGui::DragFloat("Scroll speed", &camera.scrollSpeed, 1.0f, 0.1f, 100.f, "%3.2f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::DragFloat3("Model size: ", modelSize.data(), 0.01f, 0.00001f, 10000.f, "%5.5f", ImGuiSliderFlags_AlwaysClamp);
    ImGui::Checkbox("Wireframe", &isWireframeShown);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
    vkCmdEndRenderPass(commandBuffer);
    VK_ASSERT(vkEndCommandBuffer(commandBuffer));
}

void App::createSyncObjects() {
    imageReadySemaphores.resize(imageCount);
    readyToRenderSemaphores.resize(imageCount);
    drawFrameFences.resize(imageCount);
    VkSemaphoreCreateInfo semaphoreCreateInfo{};
    semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkFenceCreateInfo fenceCreateInfo{};
    fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    for (uint32_t i = 0; i < imageCount; i++) {
        VK_ASSERT(vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageReadySemaphores[i]) ||
            vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &readyToRenderSemaphores[i]) ||
            vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFrameFences[i]));
    }
}

vector<char> App::readFile(const char* filename) {
    std::ifstream file;
    file.open(filename, std::ios_base::ate | std::ios_base::binary);
    if (!file.is_open()) {
        std::cerr << "FAILED TO OPEN A FILE!";
        VK_ASSERT(VK_ERROR_FEATURE_NOT_PRESENT);
    }
    const uint32_t size = file.tellg();
    vector<char> code(size);
    file.seekg(0);
    file.read(code.data(), size);
    file.close();
    return code;
}

VkShaderModule App::createShaderModule(const char* filename) {
    auto code = readFile(filename);
    VkShaderModule shaderModule;
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    createInfo.codeSize = static_cast<uint32_t>(code.size());
    VK_ASSERT(vkCreateShaderModule(device, &createInfo, NULL, &shaderModule));
    return shaderModule;
}

void App::loadModel(const char* obj_filename, const char* mtl_filename, VkImage* image, VkImageView* imageView, VkSampler* sampler) {
    std::ifstream obj;
    tinyobj::attrib_t attrib;
    vector<tinyobj::shape_t> shapes;
    vector<tinyobj::material_t> materials;
    std::string warn, err;
    bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_filename, mtl_filename);
    if (!warn.empty()) {
        std::cerr << warn;
    }
    if (!err.empty()) {
        std::cerr << err;
    }
    if (!res) {
        VK_ASSERT(VK_ERROR_EXTENSION_NOT_PRESENT);
        return;
    }
    for (uint32_t i = 0; i < shapes.size(); i++) {
        uint32_t index_offset = 0;
        for (uint32_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
            uint32_t fv = shapes[i].mesh.num_face_vertices[f];
            uint32_t matInd = shapes[i].mesh.material_ids[f];
            for (uint32_t v = 0; v < fv; v++) {
                tinyobj::index_t index = shapes[i].mesh.indices[v + index_offset];
                VertexData data;
                data.pos.x = attrib.vertices[3 * static_cast<uint32_t>(index.vertex_index) + 0];
                data.pos.y = attrib.vertices[3 * static_cast<uint32_t>(index.vertex_index) + 1];
                data.pos.z = attrib.vertices[3 * static_cast<uint32_t>(index.vertex_index) + 2];

                data.color.r = materials[matInd].diffuse[0];
                data.color.g = materials[matInd].diffuse[1];
                data.color.b = materials[matInd].diffuse[2];
                vertices.push_back(data);
                indices.push_back(index_offset + v);
            }
            index_offset += fv;
        }
    }
}

void App::createTexture(VkImage* image, VkImageView* imageView, VkSampler* sampler) {

}

void App::createPipelineLayout() {
    VkPushConstantRange range{};
    range.offset = 0;
    range.size = sizeof(PushConstantData);
    range.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    VkPipelineLayoutCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts = &descriptorSetLayout;
    createInfo.pPushConstantRanges = &range;
    createInfo.pushConstantRangeCount = 1;
    VK_ASSERT(vkCreatePipelineLayout(device, &createInfo, NULL, &pipelineLayout));
}

void App::createRenderPass() {
    std::vector<VkAttachmentDescription> attachmentDescriptions;
    VkAttachmentDescription attachment{};
    attachment.format = swapchainImageProperty.format;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions.push_back(attachment);

    attachment.format = depthFormat;
    attachment.samples = VK_SAMPLE_COUNT_1_BIT;
    attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachmentDescriptions.push_back(attachment);

    VkAttachmentReference colorAttachentReference{};
    colorAttachentReference.attachment = 0;
    colorAttachentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    VkAttachmentReference depthAttachmentReference{};
    depthAttachmentReference.attachment = 1;
    depthAttachmentReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpassDescriptrion{};
    subpassDescriptrion.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescriptrion.colorAttachmentCount = 1;
    subpassDescriptrion.pColorAttachments = &colorAttachentReference;
    subpassDescriptrion.pDepthStencilAttachment = &depthAttachmentReference;

    VkSubpassDependency subpassDependency{};
    subpassDependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    subpassDependency.dstSubpass = 0;
    subpassDependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    subpassDependency.srcAccessMask = NULL;
    subpassDependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    createInfo.pAttachments = attachmentDescriptions.data();
    createInfo.attachmentCount = attachmentDescriptions.size();
    createInfo.subpassCount = 1;
    createInfo.pSubpasses = &subpassDescriptrion;
    createInfo.dependencyCount = 1;
    createInfo.pDependencies = &subpassDependency;
    VK_ASSERT(vkCreateRenderPass(device, &createInfo, nullptr, &renderPass));
}

void App::createGraphicsPipelines() {
    VkShaderModule vertexShaderModule = createShaderModule("shaders/vertexShader.spv"),
        fragmentShaderModule = createShaderModule("shaders/fragmentShader.spv"),
        geometryShaderModule = createShaderModule("shaders/geometryShader.spv");
    VkPipelineShaderStageCreateInfo vertexShaderStageCreateInfo{};
    vertexShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertexShaderStageCreateInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertexShaderStageCreateInfo.module = vertexShaderModule;
    vertexShaderStageCreateInfo.pName = "main";
    VkPipelineShaderStageCreateInfo fragmentShaderStageCreateInfo{};
    fragmentShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragmentShaderStageCreateInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragmentShaderStageCreateInfo.module = fragmentShaderModule;
    fragmentShaderStageCreateInfo.pName = "main";
    VkPipelineShaderStageCreateInfo geometryShaderStageCreateInfo{};
    geometryShaderStageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    geometryShaderStageCreateInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
    geometryShaderStageCreateInfo.module = geometryShaderModule;
    geometryShaderStageCreateInfo.pName = "main";
    vector<VkPipelineShaderStageCreateInfo> shaderStages{ vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo,geometryShaderStageCreateInfo };

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = VK_FALSE;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

    auto bindings = VertexData::getBindingDescription();
    auto attributes = VertexData::getAttributeDescription();
    VkPipelineVertexInputStateCreateInfo vertexInput{};
    vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInput.pVertexAttributeDescriptions = attributes.data();
    vertexInput.vertexAttributeDescriptionCount = attributes.size();
    vertexInput.pVertexBindingDescriptions = &bindings;
    vertexInput.vertexBindingDescriptionCount = 1;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    VkViewport viewport{};
    viewport.width = swapchainImageProperty.width;
    viewport.height = swapchainImageProperty.height;
    viewport.x = 0;
    viewport.y = 0;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissors;
    scissors.offset = { 0,0 };
    scissors.extent.width = swapchainImageProperty.width;
    scissors.extent.height = swapchainImageProperty.height;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissors;

    VkPipelineRasterizationStateCreateInfo rasterization{};
    rasterization.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization.rasterizerDiscardEnable = VK_FALSE;
    rasterization.polygonMode = VK_POLYGON_MODE_FILL;
    rasterization.lineWidth = 1.0f;
    rasterization.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    rasterization.cullMode = VK_CULL_MODE_NONE;

    vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT };
    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = dynamicStates.size();
    dynamicState.pDynamicStates = dynamicStates.data();

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.blendEnable = VK_FALSE;
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_AND;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    VkPipelineDepthStencilStateCreateInfo depthCreate{};
    depthCreate.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthCreate.stencilTestEnable = VK_TRUE;
    depthCreate.depthWriteEnable = VK_TRUE;
    depthCreate.depthTestEnable = VK_TRUE;
    depthCreate.minDepthBounds = 0.f;
    depthCreate.maxDepthBounds = 1.f;
    depthCreate.depthCompareOp = VK_COMPARE_OP_LESS;
    depthCreate.back = {};
    depthCreate.front = {};
    VkGraphicsPipelineCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    createInfo.stageCount = shaderStages.size();
    createInfo.pStages = shaderStages.data();
    createInfo.layout = pipelineLayout;
    createInfo.renderPass = renderPass;
    createInfo.subpass = 0;
    createInfo.pInputAssemblyState = &inputAssembly;
    createInfo.pVertexInputState = &vertexInput;
    createInfo.pViewportState = &viewportState;
    createInfo.pRasterizationState = &rasterization;
    createInfo.pDepthStencilState = &depthCreate;
    createInfo.pDynamicState = &dynamicState;
    createInfo.pMultisampleState = &multisampling;
    createInfo.pTessellationState = nullptr;
    createInfo.pColorBlendState = &colorBlending;
    VK_ASSERT(vkCreateGraphicsPipelines(device, NULL, 1, &createInfo, NULL, &graphicsPipeline));

    vkDestroyShaderModule(device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(device, geometryShaderModule, nullptr);
}

void App::createImageView(VkImageView* imageView, VkImage image, VkFormat format, VkImageAspectFlags aspect) {
    VkImageViewCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    createInfo.image = image;
    createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    createInfo.format = format;
    createInfo.subresourceRange.aspectMask = aspect;
    createInfo.subresourceRange.layerCount = 1;
    createInfo.subresourceRange.baseArrayLayer = 0;
    createInfo.subresourceRange.levelCount = 1;
    createInfo.subresourceRange.baseMipLevel = 0;
    VK_ASSERT(vkCreateImageView(device, &createInfo, NULL, imageView));
}

void App::createBuffer(VkBuffer* buffer, VkDeviceMemory* memory, uint32_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {
    VkBufferCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.usage = usage;
    createInfo.size = size;
    VK_ASSERT(vkCreateBuffer(device, &createInfo, NULL, buffer));

    VkMemoryRequirements memReq;
    vkGetBufferMemoryRequirements(device, *buffer, &memReq);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, properties);
    VK_ASSERT(vkAllocateMemory(device, &allocInfo, NULL, memory));

    vkBindBufferMemory(device, *buffer, *memory, 0);
}

void App::createCommandPool() {
    commandBuffers.resize(imageCount);
    VkCommandPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    createInfo.queueFamilyIndex = queueFamilies.graphicsIndex.value();
    createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VK_ASSERT(vkCreateCommandPool(device, &createInfo, NULL, &commandPool));

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
    VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()));
}

void App::createVertexBuffer() {
    const uint32_t size = sizeof(VertexData) * vertices.size();
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(&stagingBuffer, &stagingBufferMemory, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* pointer;
    vkMapMemory(device, stagingBufferMemory, 0, size, NULL, &pointer);
    memcpy(pointer, vertices.data(), size);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(&vertexBuffer, &vertexBufferMemory, size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffers(stagingBuffer, vertexBuffer, size);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

void App::createIndexBuffer() {
    const uint32_t size = indices.size() * sizeof(indices[0]);
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    createBuffer(&stagingBuffer, &stagingBufferMemory, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* pointer;
    vkMapMemory(device, stagingBufferMemory, 0, size, NULL, &pointer);
    memcpy(pointer, indices.data(), size);
    vkUnmapMemory(device, stagingBufferMemory);

    createBuffer(&indexBuffer, &indexBufferMemory, size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffers(stagingBuffer, indexBuffer, size);

    vkDestroyBuffer(device, stagingBuffer, NULL);
    vkFreeMemory(device, stagingBufferMemory, NULL);
}

void App::createDescriptorTools() {
    array<VkDescriptorPoolSize, 1> poolSize{};
    poolSize[0].descriptorCount = uniformBuffers.size();
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = imageCount;
    for (auto& i : poolSize) {
        createInfo.maxSets += i.descriptorCount;
    }
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes = poolSize.data();
    VK_ASSERT(vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool));
    array<VkDescriptorSetLayoutBinding, 1> bindings{};
    bindings[0].binding = 0;
    bindings[0].descriptorCount = uniformBuffers.size();
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    VkDescriptorSetLayoutCreateInfo createInfoLayout{};
    createInfoLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfoLayout.pBindings = bindings.data();
    createInfoLayout.bindingCount = bindings.size();
    VK_ASSERT(vkCreateDescriptorSetLayout(device, &createInfoLayout, nullptr, &descriptorSetLayout));
    vector<VkDescriptorSetLayout> setLayouts(uniformBuffers.size(), descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = setLayouts.size();
    allocInfo.pSetLayouts = setLayouts.data();
    descriptorSets.resize(setLayouts.size());
    VK_ASSERT(vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()));
    for (uint32_t i = 0; i < descriptorSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        VkWriteDescriptorSet descriptroWrite{};
        descriptroWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptroWrite.descriptorCount = 1;
        descriptroWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptroWrite.dstArrayElement = 0;
        descriptroWrite.dstBinding = 0;
        descriptroWrite.dstSet = descriptorSets[i];
        descriptroWrite.pBufferInfo = &bufferInfo;
        vkUpdateDescriptorSets(device, 1, &descriptroWrite, 0, 0);
    }
}

void App::createUniformBuffers() {
    uniformBuffers.resize(imageCount);
    uniformBuffersMemory.resize(uniformBuffers.size());
    uniformBuffersMemoryPointers.resize(uniformBuffers.size());
    for (uint32_t i = 0; i < uniformBuffers.size(); i++) {
        createBuffer(&uniformBuffers[i], &uniformBuffersMemory[i], sizeof(TransformMatricesData),
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
        vkMapMemory(device, uniformBuffersMemory[i], 0, sizeof(TransformMatricesData), NULL, &uniformBuffersMemoryPointers[i]);
    }
}

void App::updateUniformBuffer(uint32_t i, float deltaTime) {
    modelMatrix = glm::scale(mat4(1.0f), vec3(modelSize[0], modelSize[1], modelSize[2]));
    TransformMatricesData data;
    data.model = modelMatrix;
    data.view = camera.getCameraView();
    data.perspective = glm::perspective(90.f, (float)swapchainImageProperty.width / (float)swapchainImageProperty.height, 0.1f, 10000.f);
    data.perspective[1][1] *= -1;
    memcpy(uniformBuffersMemoryPointers[i], &data, sizeof(TransformMatricesData));
}

void App::copyBuffers(VkBuffer srcBuffer, VkBuffer dstBuffer, uint32_t size) {
    auto commandBuffer = beginSingleCommandBuffer();
    VkBufferCopy region{};
    region.size = size;
    region.dstOffset = 0;
    region.srcOffset = 0;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);
    endSingleCommandBuffer(commandBuffer);
}

VkCommandBuffer App::beginSingleCommandBuffer() {
    VkCommandBuffer commandBuffer;
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VK_ASSERT(vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer));
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void App::endSingleCommandBuffer(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(graphicsQueue, 1, &submitInfo, NULL);
    vkDeviceWaitIdle(device);
    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

uint32_t App::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if ((1 << i) & typeFilter && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
            return i;
        }
    }
    return UINT32_MAX;
}

void App::pickPhysicalDevice() {
    uint32_t count;
    std::vector<VkPhysicalDevice> devices;
    vkEnumeratePhysicalDevices(instanceStruct.instance, &count, nullptr);
    devices.resize(count);
    vkEnumeratePhysicalDevices(instanceStruct.instance, &count, devices.data());
    for (auto& i : devices) {
        if (isPhysicalDeviceSupported(i)) {
            physicalDevice = i;
            return;
        }
    }
    std::cerr << "\nFAILED TO FIND PHYSICAL DEVICE";
    exit(1);
}

void App::createDevice() {
    pickPhysicalDevice();
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    float priority = 1.0f;
    std::array<VkDeviceQueueCreateInfo, 1> queueCreateInfos{};
    queueCreateInfos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfos[0].pQueuePriorities = &priority;
    queueCreateInfos[0].queueCount = 1;
    queueCreateInfos[0].queueFamilyIndex = queueFamilies.graphicsIndex.value();
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.enabledLayerCount = static_cast<uint32_t>(layers.size());
    createInfo.ppEnabledLayerNames = layers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();
    VkPhysicalDeviceFeatures features{};
    features.geometryShader = VK_TRUE;
    createInfo.pEnabledFeatures = &features;
    VK_ASSERT(vkCreateDevice(physicalDevice, &createInfo, NULL, &device));
    vkGetDeviceQueue(device, queueFamilies.graphicsIndex.value(), 0, &graphicsQueue);
    vkGetDeviceQueue(device, queueFamilies.presentIndex.value(), 0, &presentQueue);
    vector<VkFormat>candidates{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
    depthFormat = getSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

void App::createSwapchain() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, instanceStruct.surface, &surfaceCapabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, instanceStruct.surface, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats;
    formats.resize(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, instanceStruct.surface, &formatCount, formats.data());
    bool isFound = false;
    for (uint32_t i = 0; i < formatCount; i++) {
        if (formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR && formats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
            isFound = true;
        }
    }
    if (!isFound) {
        std::cerr << "\nFAILED TO FIND RIGHT FORMAT!";
        exit(1);
    }
    int width, height;
    glfwGetWindowSize(window, &width, &height);
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = instanceStruct.surface;
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    createInfo.preTransform = surfaceCapabilities.currentTransform;
    createInfo.minImageCount = surfaceCapabilities.minImageCount + 1;
    if (createInfo.minImageCount > surfaceCapabilities.maxImageCount) {
        createInfo.minImageCount = surfaceCapabilities.maxImageCount;
    }
    createInfo.imageColorSpace = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
    createInfo.imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    createInfo.imageExtent.width = std::min(surfaceCapabilities.maxImageExtent.width, std::max(surfaceCapabilities.minImageExtent.width, static_cast<uint32_t>(width)));
    createInfo.imageExtent.height = std::min(surfaceCapabilities.maxImageExtent.height, std::max(surfaceCapabilities.minImageExtent.height, static_cast<uint32_t>(height)));
    createInfo.imageArrayLayers = 1;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.clipped = VK_TRUE;
    VK_ASSERT(vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchain));
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapchain, &imageCount, swapchainImages.data());
    swapchainImageViews.resize(imageCount);
    swapchainImageProperty.format = createInfo.imageFormat;
    swapchainImageProperty.width = createInfo.imageExtent.width;
    swapchainImageProperty.height = createInfo.imageExtent.height;
    for (uint32_t i = 0; i < imageCount; i++) {
        createImageView(&swapchainImageViews[i], swapchainImages[i], swapchainImageProperty.format, VK_IMAGE_ASPECT_COLOR_BIT);
    }
}

bool App::isPhysicalDeviceSupported(VkPhysicalDevice physicalDevice) {
    uint32_t count;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, nullptr);
    std::vector<VkQueueFamilyProperties> queues;
    queues.resize(count);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &count, queues.data());
    for (uint32_t i = 0; i < queues.size(); i++) {
        if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            queueFamilies.graphicsIndex = i;
            queueFamilies.presentIndex = i;
        }
        if (queueFamilies.isComplete())
            return true;
    }
    return false;
}

Timer::Timer() {
    startTimePoint = lastTimePoint = std::chrono::high_resolution_clock::now();
}

float Timer::getDeltaTime() {
    auto temp = lastTimePoint;
    lastTimePoint = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<float, std::chrono::milliseconds::period>(lastTimePoint - temp).count();
}

float Timer::getProgramTime() {
    return std::chrono::duration<float, std::chrono::seconds::period>(std::chrono::high_resolution_clock::now() - startTimePoint).count();
}

VkFormat App::getSupportedFormat(vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
    for (auto format : candidates) {
        VkFormatProperties properties;
        vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &properties);
        if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) ||
            tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
            return format;
        }
    }
    VK_ASSERT(VK_ERROR_EXTENSION_NOT_PRESENT);
    exit(1);
}

void App::createDepthResources() {
    createImage(&depthImage, &depthImageMemory, swapchainImageProperty.width, swapchainImageProperty.height,
        VK_IMAGE_TILING_OPTIMAL, depthFormat, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    createImageView(&depthImageView, depthImage, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
}

bool App::hasStencilComponent(VkFormat format) {
    return format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}

void App::createImage(VkImage* image, VkDeviceMemory* memory, uint32_t width, uint32_t height, VkImageTiling tiling, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkMemoryPropertyFlagBits property) {
    VkImageCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    createInfo.usage = usage;
    createInfo.tiling = tiling;
    createInfo.samples = samples;
    createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.extent = { width,height,1 };
    createInfo.queueFamilyIndexCount = 0;
    createInfo.mipLevels = 1;
    createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    createInfo.imageType = VK_IMAGE_TYPE_2D;
    createInfo.format = format;
    createInfo.arrayLayers = 1;
    VK_ASSERT(vkCreateImage(device, &createInfo, nullptr, image));

    VkMemoryRequirements memReq;
    vkGetImageMemoryRequirements(device, *image, &memReq);
    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memReq.size;
    allocInfo.memoryTypeIndex = findMemoryType(memReq.memoryTypeBits, property);
    VK_ASSERT(vkAllocateMemory(device, &allocInfo, nullptr, memory));
    VK_ASSERT(vkBindImageMemory(device, *image, *memory, 0));
}

void App::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);
    window = glfwCreateWindow(WIDTH, HEIGHT, "DEMO 01", nullptr, nullptr);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    glfwSetWindowUserPointer(window, &camera);
    glfwSetScrollCallback(window, (GLFWscrollfun)MouseScrollCallback);
    glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)MouseButtonCallback);
    glfwSetCursorPosCallback(window, (GLFWcursorposfun)CursorPosCallback);
    glfwGetCursorPos(window, &camera.xCursorPos, &camera.yCursorPos);
}

void CursorPosCallback(GLFWwindow* window, double xPos, double yPos) {
    auto camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
    if (camera->isMiddleButtonPressed) {
        double deltaX = xPos - camera->xCursorPos;
        double deltaY = yPos - camera->yCursorPos;
        int width, height;
        glfwGetWindowSize(window, &width, &height);
        glm::vec2 deltaXY(deltaX / width, deltaY / height);
        auto mat = glm::rotate(mat4(1.f), glm::radians(360.f * deltaXY.x), vec3(0.f, 1.f, 0.f));
        camera->rotateCamera(mat);
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

mat4 Camera::getCameraView() {
    return glm::lookAt(pos, vec3(0.f, 0.f, 0.f), vec3(0.f, 1.f, 0.f));
}

void Camera::cameraScroll(double yOffset) {
    vec3 velocity = glm::normalize(pos) *static_cast<float>(yOffset) * scrollSpeed;
    vec3 temp = (pos + velocity) / pos;
    if (temp.x > 0 && temp.y > 0 && temp.z > 0) {
        pos = pos + velocity;
    }
    else {
        pos.x = temp.x > 0 ? pos.x : pos.x > 0 ? 0.1f : -0.1f;
        pos.y = temp.y > 0 ? pos.y : pos.y > 0 ? 0.1f : -0.1f;
        pos.z = temp.z > 0 ? pos.z : pos.z > 0 ? 0.1f : -0.1f;
    }
}

void Camera::rotateCamera(mat4& mat) {
    pos = vec4(pos,1.f) * mat;
}

vec3 Camera::getPos() {
    return pos;
}

void App::init() {
    initWindow();
    initVulkan();
}

void App::run() {
    init();
    mainLoop();
    appShutdown();
}

int main()
{
    App app;
    app.run();
}

