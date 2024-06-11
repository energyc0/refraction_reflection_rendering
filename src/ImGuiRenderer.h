#pragma once
#include "VulkanUtilities.h"
#include "RendererBase.h"

class ImGuiRenderer : public RendererBase{
	VulkanInstance* pVkInst;
public:
	ImGuiRenderer(VulkanInstance& VkInst, const VulkanRenderDevice& VkDev, GLFWwindow* window);
	~ImGuiRenderer();
	void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage);
	void cleanupSwapchainComponents();
	void recreateSwapchainComponents(const VulkanRenderDevice& VkDev) ;
private:
	void createDescriptorTools(const VulkanRenderDevice& VkDev);
};