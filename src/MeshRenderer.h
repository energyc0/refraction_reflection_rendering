#pragma once
#include "VulkanUtilities.h"
#include "RendererBase.h"

class MeshRenderer : RendererBase {
private:
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
	glm::mat4 model;
	glm::vec3 size;
	uint32_t indexCount;
public:
	MeshRenderer(const VulkanRenderDevice& VkDev, std::vector<ModelFilename> modelFilenames);
	~MeshRenderer();
	//void pushModel(const char* filename);
	void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage);
	void updateUniformBuffers(const Camera& camera, float deltaTime, uint32_t currentImage);
	void cleanupSwapchainComponents();
	void recreateSwapchainComponents(const VulkanRenderDevice& VkDev);
private:
	void createVertexBuffer(const VulkanRenderDevice& VkDev, std::vector<VertexData>& vertices);
	void createIndexBuffer(const VulkanRenderDevice& VkDev, std::vector<uint32_t>& indices);
	void createPipeline(const VulkanRenderDevice& VkDev);
};