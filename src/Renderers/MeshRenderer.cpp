#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(const VulkanRenderDevice& VkDev, std::vector<ModelFilename> filenames, glm::vec3& _size, VulkanTexture text) :
    RendererBase(VkDev),
    size(_size),
    texture(text){
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	for (auto& filename : filenames) {
		loadModel(filename, vertices, indices);
	}
    indexCount = static_cast<uint32_t>(indices.size());
	createVertexBuffer(VkDev,vertices);
	createIndexBuffer(VkDev,indices);
	createUniformBuffers(VkDev, sizeof(TransformMatricesData));
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = texture.imageInfo.imageView;
    imageInfo.sampler = texture.sampler;
    createDescriptorTools(VkDev, &imageInfo);
    createPipelineLayout(sizeof(PushConstantData), VK_SHADER_STAGE_FRAGMENT_BIT);
    createPipeline(VkDev);
    pushData = {};
}
MeshRenderer::~MeshRenderer() {
    vkDestroyBuffer(device, vertexBuffer.buffer, NULL);
    vkDestroyBuffer(device, indexBuffer.buffer, NULL);
    vkFreeMemory(device, vertexBuffer.memory, NULL);
    vkFreeMemory(device, indexBuffer.memory, NULL);
}
void MeshRenderer::createVertexBuffer(const VulkanRenderDevice& VkDev, std::vector<VertexData>& vertices) {
    VulkanBuffer stagingBuffer;
    stagingBuffer.size = sizeof(VertexData) * vertices.size();
    createBuffer(VkDev,
        stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vkMapMemory(device, stagingBuffer.memory, 0, stagingBuffer.size, NULL, &stagingBuffer.pointer);
    memcpy(stagingBuffer.pointer, vertices.data(), stagingBuffer.size);
    vkUnmapMemory(device, stagingBuffer.memory);

    vertexBuffer.size = stagingBuffer.size;
    createBuffer(VkDev,
        vertexBuffer, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffers(VkDev,stagingBuffer.buffer,vertexBuffer.buffer,stagingBuffer.size);

    vkDestroyBuffer(device, stagingBuffer.buffer, NULL);
    vkFreeMemory(device, stagingBuffer.memory, NULL);
}
void MeshRenderer::createIndexBuffer(const VulkanRenderDevice& VkDev, std::vector<uint32_t>& indices) {
    VulkanBuffer stagingBuffer;
    stagingBuffer.size = sizeof(uint32_t) * indices.size();
    createBuffer(VkDev,
        stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    vkMapMemory(device, stagingBuffer.memory, 0, stagingBuffer.size, NULL, &stagingBuffer.pointer);
    memcpy(stagingBuffer.pointer, indices.data(), stagingBuffer.size);
    vkUnmapMemory(device, stagingBuffer.memory);

    indexBuffer.size = stagingBuffer.size;
    createBuffer(VkDev,
        indexBuffer,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    copyBuffers(VkDev, stagingBuffer.buffer, indexBuffer.buffer, stagingBuffer.size);

    vkDestroyBuffer(device, stagingBuffer.buffer, NULL);
    vkFreeMemory(device, stagingBuffer.memory, NULL);
}
void MeshRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime) {
    VkDeviceSize offset = 0;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, NULL);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &pushData);
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}
void MeshRenderer::updateUniformBuffers(const ApplicationOptions& options, uint32_t currentImage) {
    TransformMatricesData data;
    data.model = glm::scale(glm::mat4(1.0f), size);
    data.view = options.currentCamera->getCameraView();
    data.perspective = glm::perspective(90.f, static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight), 0.1f, 100000.f);
    data.perspective[1][1] *= -1;
    memcpy(uniformBuffers[currentImage].pointer, &data, sizeof(TransformMatricesData));
    pushData.isWireframeShown = options.mode == static_cast<int>(DrawingMode::WIREFRAME);
    pushData.isReflectionEnabled = options.mode == static_cast<int>(DrawingMode::REFLECTION);
    pushData.isRefractionEnabled = options.mode == static_cast<int>(DrawingMode::REFRACTION);
    pushData.cameraPos = options.currentCamera->getPos();
}
void MeshRenderer::createPipeline(const VulkanRenderDevice& VkDev) {
    VkShaderModule vertexShaderModule = createShaderModule(VkDev, "shaders/vertexShader.spv"),
        fragmentShaderModule = createShaderModule(VkDev, "shaders/fragmentShader.spv"),
        geometryShaderModule = createShaderModule(VkDev, "shaders/geometryShader.spv");

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(3);
    shaderStages[0] = setPipelineShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = setPipelineShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    shaderStages[2] = setPipelineShaderStage(geometryShaderModule, VK_SHADER_STAGE_GEOMETRY_BIT);
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = setPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto attribute = VertexData::getAttributeDescription();
    auto bindings = VertexData::getBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInput = setPipelineVertexInputState(attribute, bindings);
    VkPipelineViewportStateCreateInfo viewportState = setPipelineViewportState(1, 1);
    VkPipelineRasterizationStateCreateInfo rasterization = setPipelineRasterizationState();
    std::vector<VkDynamicState> dynamicStates = {  VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR } ;
    VkPipelineDynamicStateCreateInfo dynamicState = setPipelineDynamicState(dynamicStates);
    VkPipelineMultisampleStateCreateInfo multisampling = setPipelineMultisampleState();
    VkPipelineColorBlendAttachmentState colorBlendAttachment = setPipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlending = setPipelineColorBlendState(&colorBlendAttachment);
    VkPipelineDepthStencilStateCreateInfo depthCreate = setPipelineDepthStencilState(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);

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

    if (vkCreateGraphicsPipelines(VkDev.device, NULL, 1, &createInfo, NULL, &graphicsPipeline)) {
        std::cerr << "vkCreateGraphicsPipelines() - FAILED!";
        exit(EXIT_FAILURE);
    }
    vkDestroyShaderModule(VkDev.device, vertexShaderModule, nullptr);
    vkDestroyShaderModule(VkDev.device, fragmentShaderModule, nullptr);
    vkDestroyShaderModule(VkDev.device, geometryShaderModule, nullptr);
}