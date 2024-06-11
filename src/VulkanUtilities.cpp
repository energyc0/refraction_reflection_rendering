#include "VulkanUtilities.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
void createImage(const VulkanRenderDevice& VkDev,
	VulkanImage& imageInfo,
	VkImageTiling tiling,
	VkSampleCountFlagBits samples,
	VkImageUsageFlags usage,
	VkMemoryPropertyFlagBits property,
	uint32_t layersCount)
{
	VkImageCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	createInfo.usage = usage;
	createInfo.tiling = tiling;
	createInfo.samples = samples;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.extent = { imageInfo.width,imageInfo.height,1 };
	createInfo.queueFamilyIndexCount = 0;
	createInfo.mipLevels = 1;
	createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	createInfo.imageType = VK_IMAGE_TYPE_2D;
	createInfo.format = imageInfo.format;
	createInfo.arrayLayers = layersCount;
	createInfo.flags = layersCount == 6 ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : NULL;
	if (vkCreateImage(VkDev.device, &createInfo, nullptr, &imageInfo.image) != VK_SUCCESS) {
		std::cerr << "vkCreateImage() - FAILED!";
		exit(EXIT_FAILURE);
	}

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements(VkDev.device, imageInfo.image, &memReq);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(VkDev,memReq.memoryTypeBits, property);
	if (vkAllocateMemory(VkDev.device, &allocInfo, nullptr, &imageInfo.imageMemory)) {
		std::cerr << "vkAllocateMemory() - FAILED!";
		exit(EXIT_FAILURE);
	}
	if(vkBindImageMemory(VkDev.device, imageInfo.image, imageInfo.imageMemory, 0)) {
		std::cerr << "vkBindImageMemory() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
void createImageView(const VulkanRenderDevice& VkDev,
	VulkanImage& imageInfo,
	VkImageAspectFlags aspect,
	VkImageViewType imageViewType,
	uint32_t layersCount) {
	VkImageViewCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = imageInfo.image;
	createInfo.viewType = imageViewType;
	createInfo.format = imageInfo.format;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.layerCount = layersCount;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseMipLevel = 0;
	if (vkCreateImageView(VkDev.device, &createInfo, NULL, &imageInfo.imageView) != VK_SUCCESS){
		std::cerr << "vkCreateImageView() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
void createBuffer(const VulkanRenderDevice& VkDev,
	VulkanBuffer& bufferInfo,
	VkBufferUsageFlags usage,
	VkMemoryPropertyFlags properties) {
	VkBufferCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	createInfo.usage = usage;
	createInfo.size = bufferInfo.size;
	if (vkCreateBuffer(VkDev.device, &createInfo, NULL, &bufferInfo.buffer) != VK_SUCCESS) {
		std::cerr << "vkCreateBuffer() - FAILED!";
		exit(EXIT_FAILURE);
	}

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements(VkDev.device, bufferInfo.buffer, &memReq);
	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memReq.size;
	allocInfo.memoryTypeIndex = findMemoryType(VkDev, memReq.memoryTypeBits, properties);
	if (vkAllocateMemory(VkDev.device, &allocInfo, NULL, &bufferInfo.memory) != VK_SUCCESS) {
		std::cerr << "vkAllocateMemory() - FAILED!";
		exit(EXIT_FAILURE);
	}

	vkBindBufferMemory(VkDev.device, bufferInfo.buffer, bufferInfo.memory, 0);
}
void createSampler(const VulkanRenderDevice& VkDev, 
	VkSampler& sampler) {
	VkSamplerCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	createInfo.unnormalizedCoordinates = VK_FALSE;
	createInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	createInfo.mipLodBias = 0.f;
	createInfo.minLod = 0.f;
	createInfo.minFilter = VK_FILTER_LINEAR;
	createInfo.maxLod = 1.0f;
	createInfo.maxAnisotropy = 1.0f;
	createInfo.magFilter = VK_FILTER_LINEAR;
	createInfo.compareEnable = VK_FALSE;
	createInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	createInfo.anisotropyEnable = VK_FALSE;
	createInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	createInfo.addressModeV = createInfo.addressModeU;
	createInfo.addressModeW = createInfo.addressModeU;
	if (vkCreateSampler(VkDev.device, &createInfo, nullptr, &sampler) != VK_SUCCESS) {
		std::cerr << "vkCreateSampler() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
VkCommandBuffer beginSingleCommandBuffer(const VulkanRenderDevice& VkDev) {
	VkCommandBuffer commandBuffer;
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = VkDev.commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(VkDev.device, &allocInfo, &commandBuffer) != VK_SUCCESS) {
		std::cerr << "vkAllocateCommandBuffers() - FAILED!";
		exit(EXIT_FAILURE);
	}
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}
void endSingleCommandBuffer(const VulkanRenderDevice& VkDev, 
	VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	vkQueueSubmit(VkDev.graphicsQueue, 1, &submitInfo, NULL);
	vkDeviceWaitIdle(VkDev.device);
	vkFreeCommandBuffers(VkDev.device, VkDev.commandPool, 1, &commandBuffer);
}
void copyImageToBuffer(const VulkanRenderDevice& VkDev,
	VkBuffer& srcBuffer,
	VkImage& dstImage,
	uint32_t width,
	uint32_t height,
	VkDeviceSize layerSize,
	uint32_t layersCount) {
	std::vector<VkBufferImageCopy> regions;
	regions.reserve(layersCount);
	for (uint32_t i = 0; i < layersCount; i++) {

		VkBufferImageCopy region{};
		region.bufferOffset = layerSize*i;
		region.imageOffset = { 0,0,0 };
		region.bufferRowLength = width;
		region.bufferImageHeight = height;
		region.imageExtent = { width,height,1 };
		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.baseArrayLayer = i;
		region.imageSubresource.layerCount = 1;
		region.imageSubresource.mipLevel = 0;
		regions.push_back(region);
	}
	VkCommandBuffer commandBuffer = beginSingleCommandBuffer(VkDev);
	vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, regions.size(), regions.data());
	endSingleCommandBuffer(VkDev, commandBuffer);
}
void copyBuffers(const VulkanRenderDevice& VkDev, 
	VkBuffer srcBuffer,
	VkBuffer dstBuffer,
	uint32_t size) {
	auto commandBuffer = beginSingleCommandBuffer(VkDev);
	VkBufferCopy region{};
	region.size = size;
	region.dstOffset = 0;
	region.srcOffset = 0;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &region);
	endSingleCommandBuffer(VkDev,commandBuffer);
}
VkShaderModule createShaderModule(const VulkanRenderDevice& VkDev, 
	const char* filename) {
	std::ifstream file;
	file.open(filename, std::ios_base::ate | std::ios_base::binary);
	if (!file.is_open()) {
		std::cerr << "FAILED TO OPEN A FILE!";
		exit(EXIT_FAILURE);
	}
	const uint32_t size = file.tellg();
	std::vector<char> code(size);
	file.seekg(0);
	file.read(code.data(), size);
	file.close();
	VkShaderModule shaderModule;
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	createInfo.codeSize = static_cast<uint32_t>(code.size());
	if (vkCreateShaderModule(VkDev.device, &createInfo, NULL, &shaderModule)) {
		std::cerr << "vkCreateShaderModule() - FAILED!";
		exit(EXIT_FAILURE);
	}
	return shaderModule;
}
void createCubemap(const char* filename,
	VkImage* image,
	VkDeviceMemory* imageMemory,
	VkImageView* imageView,
	VkSampler* sampler) {

}
void transitionImageLayout(const VulkanRenderDevice& VkDev, 
	VkImage& image,
	VkPipelineStageFlags srcStageMask,
	VkPipelineStageFlags dstStageMask,
	VkImageLayout oldLayout,
	VkImageLayout newLayout,
	VkImageSubresourceRange subresourceRange) {
	VkImageMemoryBarrier memoryBarrier{};
	memoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	memoryBarrier.image = image;
	memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	memoryBarrier.oldLayout = oldLayout;
	memoryBarrier.newLayout = newLayout;
	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		memoryBarrier.srcAccessMask = 0;
		memoryBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		memoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		memoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	}
	else {
		std::cerr << "UNDEFINED LAYOUTS!";
		exit(EXIT_FAILURE);
	}
	memoryBarrier.subresourceRange = subresourceRange;
	VkCommandBuffer commandBuffer = beginSingleCommandBuffer(VkDev);
	vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, NULL, 0, NULL, 0, NULL, 1, &memoryBarrier);
	endSingleCommandBuffer(VkDev,commandBuffer);
}
VkFormat getSupportedFormat(const VulkanRenderDevice& VkDev, 
	std::vector<VkFormat>& candidates,
	VkImageTiling tiling,
	VkFormatFeatureFlags features) {
	for (auto format : candidates) {
		VkFormatProperties properties;
		vkGetPhysicalDeviceFormatProperties(VkDev.physicalDevice, format, &properties);
		if ((tiling == VK_IMAGE_TILING_LINEAR && (properties.linearTilingFeatures & features) == features) ||
			tiling == VK_IMAGE_TILING_OPTIMAL && (properties.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	std::cerr << "FAILED TO FIND SUPPORTED FORMAT!";
	exit(EXIT_FAILURE);
}
std::vector<const char*> VulkanInstance::getExtensions() const {
	uint32_t count;
	auto ext = glfwGetRequiredInstanceExtensions(&count);
	std::vector<const char*> result(ext, ext + count);
	if (isDebugging) {
		result.emplace_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}
	return result;
}
uint32_t findMemoryType(const VulkanRenderDevice& VkDev, 
	uint32_t typeFilter,
	VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memoryProperties;
	vkGetPhysicalDeviceMemoryProperties(VkDev.physicalDevice, &memoryProperties);
	for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++) {
		if ((1 << i) & typeFilter && ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)) {
			return i;
		}
	}
	std::cerr << "FAILED TO FIND MEMORY TYPE!";
	return UINT32_MAX;
}
bool VulkanRenderDevice::isPhysicalDeviceSupported(const VkPhysicalDevice physicalDevice){
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
bool VulkanInstance::isValidationLayersSupported(const std::vector<const char*>& layers) {
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
			return isLayersSupported = false;;
	}

	return isLayersSupported = true;
}
bool hasStencilComponent(VkFormat format) {
	return format == VK_FORMAT_D24_UNORM_S8_UINT || format == VK_FORMAT_D32_SFLOAT_S8_UINT;
}
void loadModel(ModelFilename& filenames,
	std::vector<VertexData>& vertices,
	std::vector<uint32_t>& indices) {
	std::ifstream obj;
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string warn, err;
	bool res = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, filenames.obj_filename, filenames.mtl_filename);
	if (!warn.empty()) {
		std::cerr << warn;
	}
	if (!err.empty()) {
		std::cerr << err;
	}
	if (!res) {
		std::cerr << "\nFAILED TO LOAD A MODEL!";
		exit(EXIT_FAILURE);
	}
	for (uint32_t i = 0; i < shapes.size(); i++) {
		uint32_t index_offset = 0;
		for (uint32_t f = 0; f < shapes[i].mesh.num_face_vertices.size(); f++) {
			uint32_t fv = shapes[i].mesh.num_face_vertices[f];
			uint32_t matInd = shapes[i].mesh.material_ids[f];
			for (uint32_t v = 0; v < fv; v++) {
				tinyobj::index_t index = shapes[i].mesh.indices[v + index_offset];
				VertexData data{};
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

void createGraphicsPipeline(const VulkanRenderDevice& VkDev,
	const char* vertex_spv_filename,
	const char* fragment_spv_filename,
	const VkPipelineLayout& pipelineLayout,
	VkPipeline& pipeline,
	std::vector<VkVertexInputAttributeDescription> attributeDescription,
	std::vector<VkVertexInputBindingDescription> bindingDescription, 
	uint32_t width, uint32_t height,
	VkCullModeFlags cullMode,
	VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable) {

	VkShaderModule vertexShaderModule = createShaderModule(VkDev, vertex_spv_filename),
		fragmentShaderModule = createShaderModule(VkDev, fragment_spv_filename);
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
	std::array<VkPipelineShaderStageCreateInfo,2> shaderStages{ vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo};

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.pVertexAttributeDescriptions = attributeDescription.data();
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInput.pVertexBindingDescriptions = bindingDescription.data();
	vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());

	if (width == 0) width = VkDev.swapchainInfo.width;
	if (height == 0) height = VkDev.swapchainInfo.height;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	VkViewport viewport{};
	viewport.width = width;
	viewport.height = height;
	viewport.x = 0;
	viewport.y = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissors{};
	scissors.offset = { 0,0 };
	scissors.extent.width = width;
	scissors.extent.height = height;
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
	rasterization.cullMode = cullMode;

	std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT };
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
	depthCreate.depthWriteEnable = depthWriteEnable;
	depthCreate.depthTestEnable = depthTestEnable;
	depthCreate.minDepthBounds = 0.f;
	depthCreate.maxDepthBounds = 1.f;
	depthCreate.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthCreate.back = {};
	depthCreate.front = {};
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.layout = pipelineLayout;
	createInfo.renderPass = VkDev.renderPass;
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

	if (vkCreateGraphicsPipelines(VkDev.device, NULL, 1, &createInfo, NULL, &pipeline)!= VK_SUCCESS) {
		std::cerr << "vkCreateGraphicsPipelines() - FAILED!";
	}
	vkDestroyShaderModule(VkDev.device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(VkDev.device, fragmentShaderModule, nullptr);
}
void createGraphicsPipeline(const VulkanRenderDevice& VkDev,
	const char* vertex_spv_filename,
	const char* fragment_spv_filename,
	const char* geometry_spv_filename,
	const VkPipelineLayout& pipelineLayout,
	VkPipeline& pipeline,
	std::vector<VkVertexInputAttributeDescription> attributeDescription,
	std::vector<VkVertexInputBindingDescription> bindingDescription,
	uint32_t width, uint32_t height,
	VkCullModeFlags cullMode,
	VkBool32 depthTestEnable,
	VkBool32 depthWriteEnable) {
	std::vector<VkGraphicsPipelineCreateInfo> createInfos;

	VkShaderModule vertexShaderModule = createShaderModule(VkDev,vertex_spv_filename),
		fragmentShaderModule = createShaderModule(VkDev,fragment_spv_filename),
		geometryShaderModule = createShaderModule(VkDev,geometry_spv_filename);
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
	std::vector<VkPipelineShaderStageCreateInfo> shaderStages{ vertexShaderStageCreateInfo, fragmentShaderStageCreateInfo,geometryShaderStageCreateInfo };

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.primitiveRestartEnable = VK_FALSE;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

	VkPipelineVertexInputStateCreateInfo vertexInput{};
	vertexInput.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInput.pVertexAttributeDescriptions = attributeDescription.data();
	vertexInput.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescription.size());
	vertexInput.pVertexBindingDescriptions = bindingDescription.data();
	vertexInput.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescription.size());

	if (width == 0) width = VkDev.swapchainInfo.width;
	if (height == 0) height = VkDev.swapchainInfo.height;

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	VkViewport viewport{};
	viewport.width = width;
	viewport.height = height;
	viewport.x = 0;
	viewport.y = 0;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissors{};
	scissors.offset = { 0,0 };
	scissors.extent.width = width;
	scissors.extent.height = height;
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
	rasterization.cullMode = cullMode;

	std::vector<VkDynamicState> dynamicStates{ VK_DYNAMIC_STATE_VIEWPORT };
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
	depthCreate.depthWriteEnable = depthWriteEnable;
	depthCreate.depthTestEnable = depthTestEnable;
	depthCreate.minDepthBounds = 0.f;
	depthCreate.maxDepthBounds = 1.f;
	depthCreate.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depthCreate.back = {};
	depthCreate.front = {};
	VkGraphicsPipelineCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	createInfo.stageCount = shaderStages.size();
	createInfo.pStages = shaderStages.data();
	createInfo.layout = pipelineLayout;
	createInfo.renderPass = VkDev.renderPass;
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
	createInfos.push_back(createInfo);

	if(vkCreateGraphicsPipelines(VkDev.device, NULL, createInfos.size(), createInfos.data(), NULL, &pipeline)) {
		std::cerr << "vkCreateGraphicsPipelines() - FAILED!";
		exit(EXIT_FAILURE);
	}
	vkDestroyShaderModule(VkDev.device, vertexShaderModule, nullptr);
	vkDestroyShaderModule(VkDev.device, fragmentShaderModule, nullptr);
	vkDestroyShaderModule(VkDev.device, geometryShaderModule, nullptr);
}
#ifdef DEBUG
constexpr bool debugProject = true;
#else
constexpr bool debugProject = false;
#endif

static VKAPI_ATTR VkBool32 VKAPI_CALL debugMessengerCallback(VkDebugUtilsMessageTypeFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageTypes,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	std::cerr << "\nVALIDATION LAYER: " << pCallbackData->pMessage;
	return VK_FALSE;
}
//
// 
// 
// VulkanInstance
//
//
//
VulkanInstance::VulkanInstance(const GLFWwindow* window, const char* pApplicationName, const char* pEngineName, const std::vector<const char*> layers) : isDebugging(true) {
	createInstance(window, pApplicationName, pEngineName, layers);
}
VulkanInstance::~VulkanInstance() {
	if (isDebugging && isLayersSupported)
		destroyDebugUtilsMessenger(debugMessenger);
	vkDestroyInstance(instance, nullptr);
}
void VulkanInstance::createInstance(const GLFWwindow* window, const char* pApplicationName, const char* pEngineName, const std::vector<const char*>& layers) {
	VkInstanceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	if (isDebugging && isValidationLayersSupported(layers)) {
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
	appInfo.pApplicationName = pApplicationName;
	appInfo.pEngineName = pEngineName;
	createInfo.pApplicationInfo = &appInfo;
	if (vkCreateInstance(&createInfo, NULL, &instance)) {
		std::cerr << "vkCreateInstance() - FAILED!";
		exit(EXIT_FAILURE);
	}

	if (isDebugging) {
		if (createDebugUtilsMessenger((VkDebugUtilsMessengerCreateInfoEXT*)createInfo.pNext) != VK_SUCCESS) {
			std::cerr << "vkGetInstanceProcAddr() - FAILED TO FIND vkCreateDebugUtilsMessengerEXT()!\nValidation layers are disabled!\n";
		}
	}
	if (glfwCreateWindowSurface(instance,const_cast<GLFWwindow*>(window), NULL, &surface) != VK_SUCCESS) {
		std::cerr << "glfwCreateWindowSurface() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
VkResult VulkanInstance::createDebugUtilsMessenger(VkDebugUtilsMessengerCreateInfoEXT* createInfo) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if(func != nullptr){
		return func(instance, createInfo, NULL, &debugMessenger);
	}
	return VK_ERROR_EXTENSION_NOT_PRESENT;
}
void VulkanInstance::destroyDebugUtilsMessenger(VkDebugUtilsMessengerEXT messenger) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func == nullptr) {
		std::cerr << "vkGetInstanceProcAddr() - FAILED TO FIND vkDestroyDebugUtilsMessengerEXT()!\nDebugUtilsMessengerEXT was not destroyed!\n";
	}
	else {
		func(instance, messenger, NULL);
	}
}
//
// 
// 
// VulkanRenderDevice
//
// 
// 
VulkanRenderDevice::VulkanRenderDevice(const VulkanInstance& VkInst,const std::vector<const char*>& layers, GLFWwindow* window) {
	createDevice(VkInst, layers);
	createSwapchain(VkInst, window);
	createCommandPool();
	createDepthResources();
	createRenderPass();
	createFrameBuffers();
	createSyncObjects();
}
VulkanRenderDevice::~VulkanRenderDevice() {
	cleanupSwapchain();
	for (uint32_t i = 0; i < swapchainInfo.imageCount; i++) {
		vkDestroySemaphore(device, readyToRenderSemaphores[i], nullptr);
		vkDestroySemaphore(device, imageReadySemaphores[i], nullptr);
		vkDestroyFence(device, drawFrameFences[i], nullptr);
	}
	vkDestroyCommandPool(device, commandPool, nullptr);
	vkDestroyDevice(device, nullptr);
}
void VulkanRenderDevice::cleanupSwapchain() {
	vkFreeMemory(device, depthImage.imageMemory, nullptr);
	vkDestroyImage(device, depthImage.image, nullptr);
	vkDestroyImageView(device, depthImage.imageView, nullptr);
	for (uint32_t i = 0; i < swapchainInfo.imageCount; i++) {
		vkDestroyFramebuffer(device, framebuffers[i], nullptr);
		vkDestroyImageView(device, swapchainInfo.imageViews[i], nullptr);
	}
	vkDestroyRenderPass(device, renderPass, nullptr);
	vkDestroySwapchainKHR(device, swapchainInfo.swapchain, nullptr);
}
void VulkanRenderDevice::recreateSwapchain(const VulkanInstance& VkInst, GLFWwindow* window) {
	createDepthResources();
	createSwapchain(VkInst, window);
	createRenderPass();
	createFrameBuffers();
}
void VulkanRenderDevice::pickPhysicalDevice(const VulkanInstance& VkInst) {
	uint32_t count;
	std::vector<VkPhysicalDevice> devices;
	vkEnumeratePhysicalDevices(VkInst.instance, &count, nullptr);
	devices.resize(count);
	vkEnumeratePhysicalDevices(VkInst.instance, &count, devices.data());
	for (auto& i : devices) {
		if (isPhysicalDeviceSupported(i)) {
			physicalDevice = i;
			return;
		}
	}
	std::cerr << "\nFAILED TO FIND PHYSICAL DEVICE";
	exit(EXIT_FAILURE);
}
void VulkanRenderDevice::createDevice(const VulkanInstance& VkInst, const std::vector<const char*>& layers) {
	pickPhysicalDevice(VkInst);
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
	const std::vector<const char*> extensions{ VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();
	VkPhysicalDeviceFeatures features{};
	features.geometryShader = VK_TRUE;
	createInfo.pEnabledFeatures = &features;
	if (vkCreateDevice(physicalDevice, &createInfo, NULL, &device)) {
		std::cerr << "vkCreateDevice() - FAILED!";
		exit(EXIT_FAILURE);
	}
	vkGetDeviceQueue(device, queueFamilies.graphicsIndex.value(), 0, &graphicsQueue);
	vkGetDeviceQueue(device, queueFamilies.presentIndex.value(), 0, &presentQueue);
	//vector<VkFormat>candidates{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	//depthFormat = getSupportedFormat(candidates, VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
void VulkanRenderDevice::createSwapchain(const VulkanInstance& VkInst, GLFWwindow* window) {
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, VkInst.surface, &surfaceCapabilities);
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, VkInst.surface, &formatCount, nullptr);
	std::vector<VkSurfaceFormatKHR> formats;
	formats.resize(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, VkInst.surface, &formatCount, formats.data());
	bool isFound = false;
	// TO DO:
	//	PICK RIGHT FORMAT FOR USER
	//
	for (uint32_t i = 0; i < formatCount; i++) {
		if (formats[i].colorSpace == VK_COLORSPACE_SRGB_NONLINEAR_KHR && formats[i].format == VK_FORMAT_R8G8B8A8_SRGB) {
			isFound = true;
		}
	}
	if (!isFound) {
		std::cerr << "\nFAILED TO FIND RIGHT FORMAT!";
		exit(EXIT_FAILURE);
	}
	int width, height;
	glfwGetWindowSize(window, &width, &height);
	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = VkInst.surface;
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
	if (vkCreateSwapchainKHR(device, &createInfo, NULL, &swapchainInfo.swapchain) != VK_SUCCESS) {
		std::cerr << "vkCreateSwapchainKHR() - FAILED!";
		exit(EXIT_FAILURE);
	}
	vkGetSwapchainImagesKHR(device, swapchainInfo.swapchain, &swapchainInfo.imageCount, nullptr);
	swapchainInfo.images.resize(swapchainInfo.imageCount);
	vkGetSwapchainImagesKHR(device, swapchainInfo.swapchain, &swapchainInfo.imageCount, swapchainInfo.images.data());
	swapchainInfo.format = createInfo.imageFormat;
	swapchainInfo.width = createInfo.imageExtent.width;
	swapchainInfo.height = createInfo.imageExtent.height;
	swapchainInfo.imageViews.resize(swapchainInfo.imageCount);
	for (uint32_t i = 0; i < swapchainInfo.imageCount; i++) {
		VulkanImage vulkanImage{
			swapchainInfo.images[i],
			NULL,
			NULL,
			swapchainInfo.width,
			swapchainInfo.height,
			NULL,
			swapchainInfo.format
		};
		createImageView(*this, vulkanImage, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_VIEW_TYPE_2D, createInfo.imageArrayLayers);
		swapchainInfo.imageViews[i] = vulkanImage.imageView;
	}
}
void VulkanRenderDevice::createRenderPass() {
	std::vector<VkAttachmentDescription> attachmentDescriptions;
	VkAttachmentDescription attachment{};
	attachment.format = swapchainInfo.format;
	attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescriptions.push_back(attachment);

	attachment.format = depthImage.format;
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
	if (vkCreateRenderPass(device, &createInfo, nullptr, &renderPass) != VK_SUCCESS) {
		std::cerr << "vkCreateRenderPass() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
void VulkanRenderDevice::createCommandPool() {
	commandBuffers.resize(swapchainInfo.imageCount);
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilies.graphicsIndex.value();
	createInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (vkCreateCommandPool(device, &createInfo, NULL, &commandPool) != VK_SUCCESS) {
		std::cerr << "vkCreateCommandPool() - FAILED!";
		exit(EXIT_FAILURE);
	}

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = commandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());
	if (vkAllocateCommandBuffers(device, &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
		std::cerr << "vkAllocateCommandBuffers() - FAILED!";
		exit(EXIT_FAILURE);
	}
}
void VulkanRenderDevice::createFrameBuffers() {
	framebuffers.resize(swapchainInfo.imageCount);
	for (uint32_t i = 0; i < framebuffers.size(); i++) {
		std::array<VkImageView, 2> attachments{ swapchainInfo.imageViews[i], depthImage.imageView };
		VkFramebufferCreateInfo createinfo{};
		createinfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		createinfo.width = swapchainInfo.width;
		createinfo.height = swapchainInfo.height;
		createinfo.renderPass = renderPass;
		createinfo.layers = 1;
		createinfo.attachmentCount = attachments.size();
		createinfo.pAttachments = attachments.data();
		if (vkCreateFramebuffer(device, &createinfo, nullptr, &framebuffers[i]) != VK_SUCCESS) {
			std::cerr << "vkCreateFramebuffer() - FAILED!";
			exit(EXIT_FAILURE);
		}
	}
}
void VulkanRenderDevice::createSyncObjects() {
	imageReadySemaphores.resize(swapchainInfo.imageCount);
	readyToRenderSemaphores.resize(swapchainInfo.imageCount);
	drawFrameFences.resize(swapchainInfo.imageCount);
	VkSemaphoreCreateInfo semaphoreCreateInfo{};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VkFenceCreateInfo fenceCreateInfo{};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (uint32_t i = 0; i < swapchainInfo.imageCount; i++) {
		if (vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &imageReadySemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(device, &semaphoreCreateInfo, nullptr, &readyToRenderSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(device, &fenceCreateInfo, nullptr, &drawFrameFences[i]) != VK_SUCCESS) {
			std::cerr << "createSyncObjects() - FAILED!";
			exit(EXIT_FAILURE);
		}
	}
}
void VulkanRenderDevice::createDepthResources() {
	std::vector<VkFormat> candidates{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT };
	depthImage.format = getSupportedFormat(*this,
		candidates,
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	depthImage.height = swapchainInfo.height;
	depthImage.width = swapchainInfo.width;
	createImage(*this,
		depthImage,
		VK_IMAGE_TILING_OPTIMAL,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	createImageView(*this,
		depthImage,
		VK_IMAGE_ASPECT_DEPTH_BIT,VK_IMAGE_VIEW_TYPE_2D);
}
//
//
//
//VulkanQueueFamilies
//
//
//
bool VulkanQueueFamilies::isComplete() const {
	return graphicsIndex.has_value() && presentIndex.has_value();
}