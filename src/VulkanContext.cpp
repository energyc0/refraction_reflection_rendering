#include "VulkanContext.h"

bool isWindowResized = false;

const char* obj_filename = "assets/models/Duck.obj";
const char* mtl_filename = "assets/models";
const char* screenshoot_image = "C:/Users/maksi/Desktop/screenshot_image.ppm";

VulkanContext::VulkanContext(GLFWwindow* window, const char* pApplicationName, const char* pEngineName, ApplicationOptions& options) : window(window){
	VkInst = (new VulkanInstance(window,pApplicationName,pEngineName));
	VkDev = (new VulkanRenderDevice(*VkInst, window));
	createSyncObjects();
	ModelFilename filenames{};
	filenames.obj_filename = obj_filename;
	filenames.mtl_filename = mtl_filename;
	cubeRenderer = (new CubeRenderer(*VkDev));
	meshRenderer = (new MeshRenderer(*VkDev, {filenames}, options.modelSize, cubeRenderer->getCubemap()));
	imguiRenderer = (new ImGuiRenderer(*VkInst, *VkDev, window, &options));
	currentImage = 0;
}
void VulkanContext::drawFrame(ApplicationOptions& options,float deltaTime) {
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
	
	if (options.isImageToSave) {
		vkDeviceWaitIdle(VkDev->device);
		saveImage(imageIndex);
		options.isImageToSave = false;
	}
	
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
	VkDev->recreateSwapchain(*VkInst, const_cast<GLFWwindow*>(window));
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
void VulkanContext::updateUniformBuffers(const ApplicationOptions& options, float deltaTime) {
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
void VulkanContext::saveImage(uint32_t currentImage) {
	bool supportBlit = true;
	VkFormatProperties properties;
	vkGetPhysicalDeviceFormatProperties(VkDev->physicalDevice, VkDev->swapchainInfo.format, &properties);
	if (!(properties.linearTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT)) {
		std::cerr << "Blitting is not supported!\n";
		supportBlit = false;
	}
	VulkanImage stagingImage{};
	stagingImage.format = VK_FORMAT_R8G8B8A8_SRGB;
	stagingImage.height = VkDev->swapchainInfo.height;
	stagingImage.width = VkDev->swapchainInfo.width;
	VkImageSubresourceRange range{};
	range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	range.baseArrayLayer = 0;
	range.baseMipLevel = 0;
	range.layerCount = 1;
	range.levelCount = 1;
	createImage(*VkDev,
		stagingImage,
		VK_IMAGE_TILING_LINEAR,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,1);
	transitionImageLayout(*VkDev,
		stagingImage.image,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		range);
	transitionImageLayout(*VkDev,
		VkDev->swapchainInfo.images[currentImage],
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		range);
	auto commandBuffer = beginSingleCommandBuffer(*VkDev);
	if (supportBlit) {
		VkOffset3D offset;
		offset.x = stagingImage.width;
		offset.y = stagingImage.height;
		offset.z = 1;
		VkImageBlit region{};
		region.srcOffsets[1] = offset;
		region.dstOffsets[1] = offset;
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.srcSubresource.mipLevel = 0;
		region.dstSubresource = region.srcSubresource;
		vkCmdBlitImage(commandBuffer,
			VkDev->swapchainInfo.images[currentImage], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			stagingImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region,
			VK_FILTER_LINEAR);
	}
	else {
		VkImageCopy region{};
		region.extent = {VkDev->swapchainInfo.width,VkDev->swapchainInfo.height,1};
		region.srcOffset = { 0,0,0 };
		region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.srcSubresource.baseArrayLayer = 0;
		region.srcSubresource.layerCount = 1;
		region.srcSubresource.mipLevel = 0;
		region.dstOffset = { 0,0,0 };
		region.dstSubresource = region.srcSubresource;
		vkCmdCopyImage(commandBuffer,
			VkDev->swapchainInfo.images[currentImage], VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
			stagingImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1, &region);
	}
	endSingleCommandBuffer(*VkDev, commandBuffer);
	transitionImageLayout(*VkDev,
		VkDev->swapchainInfo.images[currentImage],
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		range);
	transitionImageLayout(*VkDev,
		stagingImage.image,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_MEMORY_READ_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		range);
	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(VkDev->device, stagingImage.image, &memReq);
	VkImageSubresource imgSubRes;
	imgSubRes.arrayLayer = 0;
	imgSubRes.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imgSubRes.mipLevel = 0;
	VkSubresourceLayout subresources;
	vkGetImageSubresourceLayout(VkDev->device, stagingImage.image, &imgSubRes, &subresources);

	const char* data;
	vkMapMemory(VkDev->device, stagingImage.imageMemory, 0, VK_WHOLE_SIZE, NULL, (void**)&data);
	data += subresources.offset;

	bool isSwizzled = VkDev->swapchainInfo.format == VK_FORMAT_B8G8R8_SNORM||
		VkDev->swapchainInfo.format == VK_FORMAT_B8G8R8_UNORM||
		VkDev->swapchainInfo.format == VK_FORMAT_B8G8R8_SRGB;

	std::ofstream file(screenshoot_image, std::ios_base::out | std::ios_base::binary);
	if (!file.is_open()) {
		std::cerr << "FAILED TO OPEN THE FILE!";
		exit(EXIT_FAILURE);
	}
	//file header;
	file << "P6\n" << VkDev->swapchainInfo.width << '\n' << VkDev->swapchainInfo.height << '\n' << 255 << "\n";
	const uint32_t height = VkDev->swapchainInfo.height,
		width = VkDev->swapchainInfo.width;
	for (uint32_t h = 0; h < height; h++) {
		unsigned int* row = (unsigned int*)data;
		for (uint32_t w = 0; w < width; w++) {
			if (isSwizzled) {
				file.write((const char*)row+2, 1);
				file.write((const char*)row + 1, 1);
				file.write((const char*)row, 1);
			}
			else {
				file.write((const char*)row, 3);
			}
			row++;
		}
		data += subresources.rowPitch;
	}
	file.close();
	std::cout << "Screenshot has saved: " << screenshoot_image;
	vkUnmapMemory(VkDev->device, stagingImage.imageMemory);
	vkDestroyImage(VkDev->device, stagingImage.image, nullptr);
	vkFreeMemory(VkDev->device, stagingImage.imageMemory, nullptr);
}