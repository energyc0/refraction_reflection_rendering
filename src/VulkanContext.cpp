#include "VulkanUtilities.h"
#include "MeshRenderer.h"
#include "VulkanContext.h"

const char* obj_filename = "assets/models/Duck.obj";
const char* mtl_filename = "assets/models";

VulkanContext::VulkanContext(GLFWwindow* window, const char* pApplicationName, const char* pEngineName) : window(window){
	const std::vector<const char*> layers{ "VK_LAYER_KHRONOS_validation" };
	VkInst = (new VulkanInstance(window,pApplicationName,pEngineName, layers));
	VkDev = (new VulkanRenderDevice(*VkInst,layers, window));
	ModelFilename filenames{};
	filenames.obj_filename = obj_filename;
	filenames.mtl_filename = mtl_filename;
	meshRenderer = (new MeshRenderer(*VkDev, {filenames}));
	cubeRenderer = (new CubeRenderer(*VkDev));
	imguiRenderer = (new ImGuiRenderer(*VkInst, *VkDev, window));
}
void VulkanContext::drawFrame(const Camera& camera,float deltaTime) {
	vkWaitForFences((*VkDev).device, 1, &(*VkDev).drawFrameFences[currentImage], VK_TRUE, UINT64_MAX);
	vkResetFences((*VkDev).device, 1, &(*VkDev).drawFrameFences[currentImage]);
	uint32_t imageIndex = 0;
	vkAcquireNextImageKHR((*VkDev).device,
		(*VkDev).swapchainInfo.swapchain,
		UINT64_MAX,
		(*VkDev).readyToRenderSemaphores[currentImage],
		NULL,
		&imageIndex);
	updateUniformBuffers(camera, deltaTime);
	recordCommandBuffers(deltaTime);
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &(*VkDev).commandBuffers[currentImage];
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &(*VkDev).imageReadySemaphores[currentImage];
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &(*VkDev).readyToRenderSemaphores[currentImage];
	submitInfo.pWaitDstStageMask = waitStages;
	vkQueueSubmit((*VkDev).graphicsQueue, 1,&submitInfo, (*VkDev).drawFrameFences[currentImage]);
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &(*VkDev).imageReadySemaphores[currentImage];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &(*VkDev).swapchainInfo.swapchain;
	presentInfo.pImageIndices = &imageIndex;
	vkQueuePresentKHR((*VkDev).graphicsQueue,&presentInfo);
	vkQueueWaitIdle((*VkDev).graphicsQueue);
	currentImage = (currentImage + 1) % (*VkDev).swapchainInfo.imageCount;
}
VulkanContext::~VulkanContext() {
	vkDeviceWaitIdle(VkDev->device);
	delete cubeRenderer;
	delete meshRenderer;
	delete VkDev;
	delete VkInst;
}
void VulkanContext::recreateSwapchain() {
	cleanupSwapchain();
	VkDev->recreateSwapchain(*VkInst,const_cast<GLFWwindow*>(window));
	meshRenderer->recreateSwapchainComponents(*VkDev);
	cubeRenderer->recreateSwapchainComponents(*VkDev);
	imguiRenderer->recreateSwapchainComponents(*VkDev);
}
void VulkanContext::cleanupSwapchain() {
	vkDeviceWaitIdle((*VkDev).device);
	VkDev->cleanupSwapchain();
	meshRenderer->cleanupSwapchainComponents();
	cubeRenderer->cleanupSwapchainComponents();
	imguiRenderer->cleanupSwapchainComponents();
}
void VulkanContext::updateUniformBuffers(const Camera& camera ,float deltaTime) {
	meshRenderer->updateUniformBuffers(camera, deltaTime, currentImage);
	cubeRenderer->updateUniformBuffers(camera, deltaTime, currentImage);
}
void VulkanContext::recordCommandBuffers(float deltaTime) {
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	VkRenderPassBeginInfo renderPassBegin{};
	renderPassBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassBegin.renderPass = (*VkDev).renderPass;
	renderPassBegin.framebuffer = (*VkDev).framebuffers[currentImage];
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.3f,0.3f,0.3f,1.0f };
	clearValues[1].depthStencil = { 1.0f,0 };
	renderPassBegin.pClearValues = clearValues.data();
	renderPassBegin.clearValueCount = clearValues.size();
	renderPassBegin.renderArea.offset = { 0,0 };
	renderPassBegin.renderArea.extent = { (*VkDev).swapchainInfo.width, (*VkDev).swapchainInfo.height };
	VkDeviceSize offsets[] = { 0 };
	VkViewport viewport{};
	viewport.x = viewport.y = 0;
	viewport.width = (*VkDev).swapchainInfo.width;
	viewport.height = (*VkDev).swapchainInfo.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkCommandBuffer commandBuffer = (*VkDev).commandBuffers[currentImage];
	if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
		std::cerr << "vkBeginCommandBuffer() - FAILED";
		exit(EXIT_FAILURE);
	}
	vkCmdBeginRenderPass(commandBuffer, &renderPassBegin, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	cubeRenderer->fillCommandBuffer(commandBuffer, currentImage);
	imguiRenderer->fillCommandBuffer(commandBuffer, currentImage);
	meshRenderer->fillCommandBuffer(commandBuffer, currentImage);
	vkCmdEndRenderPass(commandBuffer);
	if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
		std::cerr << "vkEndCommandBuffer() - FAILED";
		exit(EXIT_FAILURE);
	}
}