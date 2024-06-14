#pragma once
#include "RendererBase.h"

class ImGuiRenderer : public RendererBase{
	VulkanInstance* pVkInst;
	ApplicationOptions* const options;
public:
	ImGuiRenderer(VulkanInstance& VkInst, const VulkanRenderDevice& VkDev, GLFWwindow* window, ApplicationOptions* const options);
	~ImGuiRenderer();
	void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime);
private:
	void createDescriptorTools(const VulkanRenderDevice& VkDev);
};