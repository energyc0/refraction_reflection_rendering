#include "CubeRenderer.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

CubeRenderer::CubeRenderer(VulkanRenderDevice& VkDev) : RendererBase(VkDev){
    CubemapFilenames filenames{};
    filenames.negX = "assets/textures/skybox/left.jpg";
    filenames.negY = "assets/textures/skybox/bottom.jpg";
    filenames.negZ = "assets/textures/skybox/back.jpg";
    filenames.posX = "assets/textures/skybox/right.jpg";
    filenames.posY = "assets/textures/skybox/top.jpg";
    filenames.posZ = "assets/textures/skybox/front.jpg";
    createUniformBuffers(VkDev, sizeof(TransformMatricesData));
    createCubemapTexture(VkDev, &filenames);
    createSampler(VkDev, cubemap.sampler);
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = cubemap.imageInfo.imageView;
    imageInfo.sampler = cubemap.sampler;
    createDescriptorTools(VkDev, &imageInfo);
    createPipelineLayout();
    createPipeline(VkDev);
}
CubeRenderer::~CubeRenderer() {
    vkFreeMemory(device, cubemap.imageInfo.imageMemory, nullptr);
    vkDestroySampler(device, cubemap.sampler, nullptr);
    vkDestroyImage(device, cubemap.imageInfo.image,nullptr);
    vkDestroyImageView(device, cubemap.imageInfo.imageView,nullptr);
}
void CubeRenderer::createPipeline(const VulkanRenderDevice& VkDev) {
    VkShaderModule vertexShaderModule = createShaderModule(VkDev, "shaders/skyboxVertexShader.spv"),
        fragmentShaderModule = createShaderModule(VkDev, "shaders/skyboxFragmentShader.spv");

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages(2);
    shaderStages[0] = setPipelineShaderStage(vertexShaderModule, VK_SHADER_STAGE_VERTEX_BIT);
    shaderStages[1] = setPipelineShaderStage(fragmentShaderModule, VK_SHADER_STAGE_FRAGMENT_BIT);
    VkPipelineInputAssemblyStateCreateInfo inputAssembly = setPipelineInputAssemblyState(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST);
    auto attribute = VertexData::getAttributeDescription();
    auto bindings = VertexData::getBindingDescription();
    VkPipelineVertexInputStateCreateInfo vertexInput = setPipelineVertexInputState(attribute, bindings);
    VkPipelineViewportStateCreateInfo viewportState = setPipelineViewportState(1, 1);
    VkPipelineRasterizationStateCreateInfo rasterization = setPipelineRasterizationState();
    std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
    VkPipelineDynamicStateCreateInfo dynamicState = setPipelineDynamicState(dynamicStates);
    VkPipelineMultisampleStateCreateInfo multisampling = setPipelineMultisampleState();
    VkPipelineColorBlendAttachmentState colorBlendAttachment = setPipelineColorBlendAttachmentState();
    VkPipelineColorBlendStateCreateInfo colorBlending = setPipelineColorBlendState(&colorBlendAttachment);
    VkPipelineDepthStencilStateCreateInfo depthCreate = setPipelineDepthStencilState(VK_TRUE, VK_FALSE, VK_COMPARE_OP_LESS_OR_EQUAL);

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
}
void CubeRenderer::createCubemapTexture(VulkanRenderDevice& VkDev,CubemapFilenames* filenames) {
    std::array<stbi_uc*, 6> pixels{};
    int width, height, channels;
    pixels[0] = stbi_load(filenames->posX, &width, &height, &channels, STBI_rgb_alpha);
    pixels[1] = stbi_load(filenames->negX, &width, &height, &channels, STBI_rgb_alpha);
    pixels[2] = stbi_load(filenames->posY, &width, &height, &channels, STBI_rgb_alpha);
    pixels[3] = stbi_load(filenames->negY, &width, &height, &channels, STBI_rgb_alpha);
    pixels[4] = stbi_load(filenames->posZ, &width, &height, &channels, STBI_rgb_alpha);
    pixels[5] = stbi_load(filenames->negZ, &width, &height, &channels, STBI_rgb_alpha);
    const VkDeviceSize layerSize = width * height * 4;
    VulkanBuffer stagingBuffer;
    stagingBuffer.size = layerSize * 6;
    createBuffer(VkDev,
        stagingBuffer,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    for (uint32_t i = 0; i < 6; i++) {
        vkMapMemory(device, stagingBuffer.memory, layerSize*i, layerSize, NULL, &stagingBuffer.pointer);
        memcpy(stagingBuffer.pointer, pixels[i], layerSize);
        vkUnmapMemory(device, stagingBuffer.memory);
    }
    VkImageSubresourceRange subresourceRange{};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 6;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    cubemap.desiredLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    cubemap.imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    cubemap.imageInfo.width = width;
    cubemap.imageInfo.height = height;
    createImage(VkDev,
        cubemap.imageInfo,
        VK_IMAGE_TILING_OPTIMAL,
        VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        6);
    createImageView(VkDev,
        cubemap.imageInfo,
        VK_IMAGE_ASPECT_COLOR_BIT,
        VK_IMAGE_VIEW_TYPE_CUBE,
        6);
    transitionImageLayout(VkDev,cubemap.imageInfo.image,
        VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);
    copyImageToBuffer(VkDev,
        stagingBuffer.buffer,
        cubemap.imageInfo.image,
        width, height, 
        layerSize,
        6);
    transitionImageLayout(VkDev, cubemap.imageInfo.image,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, cubemap.desiredLayout, subresourceRange);
    vkFreeMemory(device, stagingBuffer.memory, nullptr);
    vkDestroyBuffer(device, stagingBuffer.buffer, nullptr);
    createSampler(VkDev, cubemap.sampler);
}
void CubeRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage, float deltaTime) {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1, &descriptorSets[currentImage], 0, NULL);
    vkCmdDraw(commandBuffer, 36, 1, 0, 0);
}
void CubeRenderer::updateUniformBuffers(const ApplicationOptions& options, uint32_t currentImage) {
    TransformMatricesData data;
    data.model = glm::mat4(1.0f);
    data.view = options.currentCamera->getCameraView();
    data.perspective = glm::perspective(90.f, static_cast<float>(framebufferWidth) / static_cast<float>(framebufferHeight), 0.1f, 100000.f);
    data.perspective[1][1] *= -1;
    memcpy(uniformBuffers[currentImage].pointer, &data, uniformBuffers[currentImage].size);
}