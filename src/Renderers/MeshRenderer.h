#pragma once
#include "RendererBase.h"

enum class DrawingMode {
	SOLID = 0,
	WIREFRAME = 1,
	REFLECTION = 2,
	REFRACTION = 3
};

class MeshRenderer : RendererBase {
private:
	VulkanBuffer vertexBuffer;
	VulkanBuffer indexBuffer;
	glm::vec3& const size;
	uint32_t indexCount;
	VulkanTexture texture;
	PushConstantData pushData;
public:
	MeshRenderer(const VulkanRenderDevice& VkDev, std::vector<ModelFilename> modelFilenames, glm::vec3& size, VulkanTexture text);
	~MeshRenderer();
	//void pushModel(const char* filename);
	void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime);
	void updateUniformBuffers(const ApplicationOptions& options, uint32_t currentImage);
private:
	void createVertexBuffer(const VulkanRenderDevice& VkDev, std::vector<VertexData>& vertices);
	void createIndexBuffer(const VulkanRenderDevice& VkDev, std::vector<uint32_t>& indices);
	void createPipeline(const VulkanRenderDevice& VkDev);
};