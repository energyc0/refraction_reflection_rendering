#pragma once
#include "RendererBase.h"

struct CubemapFilenames {
	const char* posX;
	const char* negX;
	const char* posY;
	const char* negY;
	const char* posZ;
	const char* negZ;
};

class CubeRenderer : public RendererBase{
private:
	VulkanTexture cubemap;

public:
	CubeRenderer(VulkanRenderDevice& VkDev);
	void updateUniformBuffers(const ApplicationOptions& options, uint32_t currentImage);
	void fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime);
	inline VulkanTexture getCubemap() const { return cubemap; }
	~CubeRenderer();
private:
	void createPipeline(const VulkanRenderDevice& VkDev);
	void createCubemapTexture(VulkanRenderDevice& VkDev, CubemapFilenames* filenames);
};