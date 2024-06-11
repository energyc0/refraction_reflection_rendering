#include "MeshRenderer.h"

MeshRenderer::MeshRenderer(const VulkanRenderDevice& VkDev, std::vector<ModelFilename> filenames) : RendererBase(VkDev) {
	std::vector<VertexData> vertices;
	std::vector<uint32_t> indices;
	for (auto& filename : filenames) {
		loadModel(filename, vertices, indices);
	}
    indexCount = static_cast<uint32_t>(indices.size());
	createVertexBuffer(VkDev,vertices);
	createIndexBuffer(VkDev,indices);
	createUniformBuffers(VkDev, sizeof(TransformMatricesData));
    createDescriptorTools(VkDev);
    createPipelineLayout(sizeof(PushConstantData), VK_SHADER_STAGE_FRAGMENT_BIT);
    createPipeline(VkDev);
    model = glm::mat4(1.0f);
    size = glm::vec3(1.0f);
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
void MeshRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage) {
    VkDeviceSize offset = 0;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, NULL);
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer.buffer, &offset);
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);
    PushConstantData data;
    data.isWireframeShown = true;
    data.sinus = 1.0f;
    vkCmdPushConstants(commandBuffer, pipelineLayout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstantData), &data);
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
}
void MeshRenderer::updateUniformBuffers(const Camera& camera, float deltaTime, uint32_t currentImage) {
    model = glm::scale(glm::mat4(1.0f), glm::vec3(1.0f));
    TransformMatricesData data;
    data.model = model;
    data.view = camera.getCameraView();
    data.perspective = glm::perspective(90.f, static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight), 0.1f, 100000.f);
    data.perspective[1][1] *= -1;
    memcpy(uniformBuffers[currentImage].pointer, &data, sizeof(TransformMatricesData));
}
void MeshRenderer::createPipeline(const VulkanRenderDevice& VkDev) {
    createGraphicsPipeline(VkDev,
        "shaders/vertexShader.spv",
        "shaders/fragmentShader.spv",
        "shaders/geometryShader.spv",
        pipelineLayout,
        graphicsPipeline);
}
void MeshRenderer::cleanupSwapchainComponents() {
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
}
void MeshRenderer::recreateSwapchainComponents(const VulkanRenderDevice& VkDev) {
    cleanupSwapchainComponents();
    framebufferWidth = VkDev.swapchainInfo.width;
    framebufferHeight = VkDev.swapchainInfo.height;
    createPipeline(VkDev);
}