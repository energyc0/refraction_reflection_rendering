#pragma once
#include "VulkanUtilities.h"
#include "MeshRenderer.h"
#include "CubeRenderer.h"
#include "ImGuiRenderer.h"
#include <memory>

class VulkanContext {
private:
	const GLFWwindow* const window;
	VulkanInstance* VkInst;
	VulkanRenderDevice*VkDev;
	MeshRenderer *meshRenderer;
	CubeRenderer* cubeRenderer;
	ImGuiRenderer* imguiRenderer;
	uint32_t currentImage = 0;
public:
	VulkanContext(GLFWwindow* window, const char* pApplicationName, const char* pEngineName);
	void drawFrame(const Camera& camera, float deltaTime);
	~VulkanContext();
private:
	void recreateSwapchain();
	void cleanupSwapchain();
	void recordCommandBuffers(float deltaTime);
	void updateUniformBuffers(const Camera& camera, float deltaTime);
};