#include "VulkanContext.h"

bool isWindowResized = false;

const char* obj_filename = "assets/models/Duck.obj";
const char* mtl_filename = "assets/models";

VulkanContext::VulkanContext(GLFWwindow* window, const char* pApplicationName, const char* pEngineName, ApplicationOptions& options) : window(window){
	VkInst = (new VulkanInstance(window,pApplicationName,pEngineName));
	VkDev = (new VulkanRenderDevice(*VkInst, window));
	createSyncObjects();
	ModelFilename filenames{};
	filenames.obj_filename = obj_filename;
	filenames.mtl_filename = mtl_filename;
	meshRenderer = (new MeshRenderer(*VkDev, {filenames}, options.modelSize));
	cubeRenderer = (new CubeRenderer(*VkDev));
	imguiRenderer = (new ImGuiRenderer(*VkInst, *VkDev, window, &options));
	currentImage = 0;
}
void VulkanContext::drawFrame(const ApplicationOptions& options,float deltaTime) {
	vkWaitForFences(VkDev->device, 1, &drawFrameFences[currentImage], VK_TRUE, UINT64_MAX);
	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(VkDev->device,
		VkDev->swapchainInfo.swapchain,
		UINT64_MAX,
		readyToRenderSemaphores[currentImage],
		NULL,
		&imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		recreateSwapchain();
		return;
	}
	else if (result != VK_SUCCESS) {
		std::cerr << "vkAcquireNextImageKHR() - FAILED!";
		exit(EXIT_FAILURE);
	}
	vkResetFences(VkDev->device, 1, &drawFrameFences[currentImage]);

	updateUniformBuffers(options, deltaTime);
	recordCommandBuffers(deltaTime);
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &VkDev->commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &imageReadySemaphores[currentImage];
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &readyToRenderSemaphores[currentImage];
	submitInfo.pWaitDstStageMask = waitStages;
	result = vkQueueSubmit(VkDev->graphicsQueue, 1,&submitInfo, drawFrameFences[currentImage]);
	if (result != VK_SUCCESS) {
		std::cerr << "vkQueueSubmit() - FAILED!";
		exit(EXIT_FAILURE);
	}
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &imageReadySemaphores[currentImage];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &VkDev->swapchainInfo.swapchain;
	presentInfo.pImageIndices = &imageIndex;
	result = vkQueuePresentKHR(VkDev->graphicsQueue,&presentInfo);
	if (isWindowResized || result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
		isWindowResized = false;
		recreateSwapchain();
	}
	else if (result != VK_SUCCESS) {
		std::cerr << "vkQueuePresentKHR() - FAILED!";
		exit(EXIT_FAILURE);
	}
	vkQueueWaitIdle(VkDev->graphicsQueue);
	currentImage = (currentImage + 1) % VkDev->swapchainInfo.imageCount;
}
VulkanContext::~VulkanContext() {
	vkDeviceWaitIdle(VkDev->device);
	for (uint32_t i = 0; i < VkDev->swapchainInfo.imageCount; i++) {
		vkDestroySemaphore(VkDev->device, readyToRenderSemaphores[i], nullptr);
		vkDestroySemaphore(VkDev->device, imageReadySemaphores[i], nullptr);
		vkDestroyFence(VkDev->device, drawFrameFences[i], nullptr);
	}
	delete cubeRenderer;
	delete meshRenderer;
	delete VkDev;
	delete VkInst;
}
void VulkanContext::recreateSwapchain() {
	cleanupSwapchain();
	VkDev->recreateSwapchain(*VkInst,const_cast<GLFWwindow*>(window));
}
void VulkanContext::cleanupSwapchain() {
	vkDeviceWaitIdle(VkDev->device);
	int width, height;
	glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &width, &height);
	while (width == 0 || height == 0) {
		glfwGetFramebufferSize(const_cast<GLFWwindow*>(window), &width, &height);
		glfwWaitEvents();
	}
	VkDev->cleanupSwapchain();
}
void VulkanContext::updateUniformBuffers(const ApplicationOptions& options,float deltaTime) {
	meshRenderer->updateUniformBuffers(options, currentImage);
	cubeRenderer->updateUniformBuffers(options, currentImage);
}
void VulkanContext::recordCommandBuffers(float deltaTime) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VkRenderPassBeginInfo renderPassBegin{};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.renderPass = VkDev->renderPass;
	renderPassBegin.framebuffer = VkDev->framebuffers[currentImage];
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.3f,0.3f,0.3f,1.0f };
	clearValues[1].depthStencil = { 1.0f,0 };
	renderPassBegin.pClearValues = clearValues.data();
	renderPassBegin.clearValueCount = clearValues.size();
	renderPassBegin.renderArea.offset = { 0,0 };
	renderPassBegin.renderArea.extent = { VkDev->swapchainInfo.width, VkDev->swapchainInfo.height };
	VkDeviceSize offsets[] = { 0 };
	VkViewport viewport{};
	viewport.x = viewport.y = 0;
	viewport.width = VkDev->swapchainInfo.width;
	viewport.height = VkDev->swapchainInfo.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkCommandBuffer commandBuffer = VkDev->commandBuffers[currentImage];
	VkRect2D scissor{};
	scissor.offset = { 0,0 };
	scissor.extent = { VkDev->swapchainInfo.width, VkDev->swapchainInfo.height };
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		std::cerr << "vkBeginCommandBuffer() - FAILED";
		exit(EXIT_FAILURE);
	}
	vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	meshRenderer->fillCommandBuffer(commandBuffer, currentImage, deltaTime);
	cubeRenderer->fillCommandBuffer(commandBuffer, currentImage, deltaTime);
	imguiRenderer->fillCommandBuffer(commandBuffer, currentImage, deltaTime);
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		std::cerr << "vkEndCommandBuffer() - FAILED";
		exit(EXIT_FAILURE);
	}
}
void VulkanContext::createSyncObjects() {
	imageReadySemaphores.resize(VkDev->swapchainInfo.imageCount);
	readyToRenderSemaphores.resize(VkDev->swapchainInfo.imageCount);
	drawFrameFences.resize(VkDev->swapchainInfo.imageCount);
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (uint32_t i = 0; i < VkDev->swapchainInfo.imageCount; i++) {
		if (vkCreateSemaphore(VkDev->device, &semaphoreCreateInfo, nullptr, &imageReadySemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(VkDev->device, &semaphoreCreateInfo, nullptr, &readyToRenderSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(VkDev->device, &fenceCreateInfo, nullptr, &drawFrameFences[i]) != VK_SUCCESS) {
			std::cerr << "createSyncObjects() - FAILED!";
			exit(EXIT_FAILURE);
		}
	}
}