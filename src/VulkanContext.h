#pragma once
#include "VulkanUtilities.h"
#include "MeshRenderer.h"
#include "CubeRenderer.h"
#include "ImGuiRenderer.h"

class VulkanContext {
private:
	const GLFWwindow* const window;
	VulkanInstance* VkInst;
	VulkanRenderDevice*VkDev;
	MeshRenderer *meshRenderer;
	CubeRenderer* cubeRenderer;
	ImGuiRenderer* imguiRenderer;
	uint32_t currentImage;

	std::vector<VkSemaphore> imageReadySemaphores;
	std::vector<VkSemaphore> readyToRenderSemaphores;
	std::vector<VkFence> drawFrameFences;
public:
	VulkanContext(GLFWwindow* window, const char* pApplicationName, const char* pEngineName, ApplicationOptions& options);
	void drawFrame(const ApplicationOptions& options, float deltaTime);
	~VulkanContext();
private:
	void createSyncObjects();
	void recreateSwapchain();
	void cleanupSwapchain();
	void recordCommandBuffers(float deltaTime);
	void updateUniformBuffers(const ApplicationOptions& options, float deltaTime);
};