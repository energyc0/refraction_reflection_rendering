#pragma once
#include "VulkanUtilities.h"

class RendererBase {
protected:
	VkDevice device = VK_NULL_HANDLE;

	VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
	VkPipeline graphicsPipeline = VK_NULL_HANDLE;

	VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::vector<VkDescriptorSet> descriptorSets;

	std::vector<VulkanBuffer> uniformBuffers;

	uint32_t framebufferWidth = 0;
	uint32_t framebufferHeight = 0;
public:
	explicit RendererBase(const VulkanRenderDevice& VkDev) : 
		device(VkDev.device),
		framebufferWidth(VkDev.swapchainInfo.width),
		framebufferHeight(VkDev.swapchainInfo.height){};
	virtual ~RendererBase();
	virtual void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime) = 0;
	virtual void updateUniformBuffers(const ApplicationOptions& options, uint32_t currentImage) {};
protected:
	bool createUniformBuffers(const VulkanRenderDevice& VkDev, VkDeviceSize bufferSize);
	virtual void createDescriptorTools(const VulkanRenderDevice& VkDev, VkDescriptorImageInfo* imageInfo = nullptr);
	virtual void createPipelineLayout(uint32_t pushConstantSize = 0, VkShaderStageFlags pushConstantStage = NULL);
};