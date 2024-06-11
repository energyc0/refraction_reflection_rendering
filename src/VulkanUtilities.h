#pragma once
#include <iostream>
#include <fstream>
#include <optional>
#include <array>
#include <vector>
#include <vulkan/vulkan.h>
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

struct VertexData {
    glm::vec3 pos;
    alignas(16)glm::vec3 color;
    alignas(16)glm::vec3 texCoords;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription binding{};
        binding.binding = 0;
        binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        binding.stride = sizeof(VertexData);
        return binding;
    }
    static std::vector<VkVertexInputAttributeDescription> getAttributeDescription() {
        std::vector<VkVertexInputAttributeDescription>description(3);
        description[0].binding = 0;
        description[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        description[0].location = 0;
        description[0].offset = offsetof(VertexData, pos);

        description[1].binding = 0;
        description[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        description[1].location = 1;
        description[1].offset = offsetof(VertexData, color);

        description[2].binding = 0;
        description[2].format = VK_FORMAT_R32G32B32_SFLOAT;
        description[2].location = 2;
        description[2].offset = offsetof(VertexData, texCoords);
        return description;
    }
};

struct VulkanQueueFamilies {
    std::optional<uint32_t> graphicsIndex;
    std::optional<uint32_t> presentIndex;
    bool isComplete() const;
};

struct VulkanSwapchainInfo {
    VkSwapchainKHR swapchain;

    uint32_t imageCount;
    VkFormat format;
    uint32_t width;
    uint32_t height;

    std::vector<VkImage> images;
    std::vector<VkImageView> imageViews;
};

struct VulkanBuffer {
    VkBuffer buffer;
    VkDeviceMemory memory;
    VkDeviceSize size;
    void* pointer;
};

struct VulkanImage {
    VkImage image;
    VkImageView imageView;
    VkDeviceMemory imageMemory;

    uint32_t width;
    uint32_t height;
    VkDeviceSize size;
    VkFormat format;
};

struct VulkanTexture {
    VulkanImage imageInfo;
    VkImageLayout desiredLayout;
    VkSampler sampler;
};


struct ModelFilename {
    const char* obj_filename;
    const char* mtl_filename;
};

struct PushConstantData {
    float sinus;
    VkBool32 isWireframeShown;
};

struct TransformMatricesData {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 perspective;
};

class VulkanInstance {
private:
    bool isLayersSupported;
public:
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkSurfaceKHR surface;
    const bool isDebugging;
private:
    void createInstance(const GLFWwindow* window, const char* pApplicationName, const char* pEngineName, const std::vector<const char*>& layers);
    VkResult createDebugUtilsMessenger(VkDebugUtilsMessengerCreateInfoEXT* createInfo);
    void destroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT messenger);
    bool isValidationLayersSupported(const std::vector<const char*>& layers);
    std::vector<const char*> getExtensions() const;
public:
    VulkanInstance(const GLFWwindow* window, const char* pApplicationName, const char* pEngineNameconst, std::vector<const char*> layers);
    ~VulkanInstance();
};

class VulkanRenderDevice {
public:
    VkPhysicalDevice physicalDevice;
    VkDevice device;

    VulkanSwapchainInfo swapchainInfo;

    VkRenderPass renderPass;

    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;

    VulkanQueueFamilies queueFamilies;
    std::vector<VkFramebuffer> framebuffers;
    std::vector<VkSemaphore> imageReadySemaphores;
    std::vector<VkSemaphore> readyToRenderSemaphores;
    std::vector<VkFence> drawFrameFences;
    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VulkanImage depthImage;
private:
    void pickPhysicalDevice(const VulkanInstance& VkInst);
    void createDevice(const VulkanInstance& VkInst, const std::vector<const char*>& layers);
    void createSwapchain(const VulkanInstance& VkInst, GLFWwindow* window);
    void createRenderPass();
    void createCommandPool();
    void createFrameBuffers();
    void createSyncObjects();
    void createDepthResources();
    bool isPhysicalDeviceSupported(VkPhysicalDevice physicalDevice);
public:
    VulkanRenderDevice(const VulkanInstance& VkInst, const std::vector<const char*>& layers, GLFWwindow* window);
    void cleanupSwapchain();
    void recreateSwapchain(const VulkanInstance& VkInst, GLFWwindow* window);
    ~VulkanRenderDevice();
};

///
void createImage(const VulkanRenderDevice& VkDev,
    VulkanImage& imageInfo,
    VkImageTiling tiling,
    VkSampleCountFlagBits samples,
    VkImageUsageFlags usage,
    VkMemoryPropertyFlagBits property,
    uint32_t layersCount = 1);

void createImageView(const VulkanRenderDevice& VkDev,
    VulkanImage& imageInfo,
    VkImageAspectFlags aspect,
    VkImageViewType imageViewType,
    uint32_t layersCount = 1);

void createBuffer(const VulkanRenderDevice& VkDev,
    VulkanBuffer& bufferInfo,
    VkBufferUsageFlags usage,
    VkMemoryPropertyFlags properties);

VkShaderModule createShaderModule(const VulkanRenderDevice& VkDev,
    const char* filename);

void createSampler(const VulkanRenderDevice& VkDev,
    VkSampler& sampler);

VkCommandBuffer beginSingleCommandBuffer(const VulkanRenderDevice& VkDev);

void endSingleCommandBuffer(const VulkanRenderDevice& VkDev,
    VkCommandBuffer commandBuffer);

void copyImageToBuffer(const VulkanRenderDevice& VkDev,
    VkBuffer& srcBuffer,
    VkImage& dstImage,
    uint32_t width,
    uint32_t height,
    VkDeviceSize layerSize,
    uint32_t layersCount = 1);

void copyBuffers(const VulkanRenderDevice& VkDev,
    VkBuffer srcBuffer,
    VkBuffer dstBuffer,
    uint32_t size);

//void createCubemap(const char* filename, VkImage* image, VkDeviceMemory* imageMemory, VkImageView* imageView, VkSampler* sampler);

void transitionImageLayout(const VulkanRenderDevice& VkDev,
    VkImage& image,
    VkPipelineStageFlags srcStageMask,
    VkPipelineStageFlags dstStageMask,
    VkImageLayout oldLayout,
    VkImageLayout newLayout,
    VkImageSubresourceRange subresourceRange);

VkFormat getSupportedFormat(const VulkanRenderDevice& VkDev,
    std::vector<VkFormat>& candidates,
    VkImageTiling tiling,
    VkFormatFeatureFlags features);

uint32_t findMemoryType(const VulkanRenderDevice& VkDev,
    uint32_t typeFilter,
    VkMemoryPropertyFlags properties);

bool hasStencilComponent(VkFormat format);

void loadModel(ModelFilename& filenames,
    std::vector<VertexData>& vertices,
    std::vector<uint32_t>& indices);

void createGraphicsPipeline(const VulkanRenderDevice& VkDev,
    const char* vertex_spv_filename,
    const char* fragment_spv_filename,
    const VkPipelineLayout& pipelineLayout,
    VkPipeline& pipeline,
    std::vector<VkVertexInputAttributeDescription> attributeDescription = VertexData::getAttributeDescription(),
    std::vector<VkVertexInputBindingDescription> bindingDescription = { VertexData::getBindingDescription() },
    uint32_t width = 0, uint32_t height = 0,
    VkCullModeFlags cullMode = VK_CULL_MODE_BACK_BIT,
    VkBool32 depthTestEnable = VK_TRUE,
    VkBool32 depthWriteEnable = VK_TRUE);
void createGraphicsPipeline(const VulkanRenderDevice& VkDev,
    const char* vertex_spv_filename,
    const char* fragment_spv_filename,
    const char* geometry_spv_filename,
    const VkPipelineLayout& pipelineLayout,
    VkPipeline& pipeline,
    std::vector<VkVertexInputAttributeDescription> attributeDescription = VertexData::getAttributeDescription(),
    std::vector<VkVertexInputBindingDescription> bindingDescription = { VertexData::getBindingDescription() },
    uint32_t width=0, uint32_t height=0,
    VkCullModeFlags cullMode=VK_CULL_MODE_BACK_BIT,
    VkBool32 depthTestEnable = VK_TRUE,
    VkBool32 depthWriteEnable = VK_TRUE);