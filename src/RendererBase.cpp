#include "VulkanUtilities.h"
#include "RendererBase.h"

bool RendererBase::createUniformBuffers(const VulkanRenderDevice& VkDev, VkDeviceSize bufferSize) {
    uniformBuffers.resize(VkDev.swapchainInfo.imageCount);
    for (uint32_t i = 0; i < uniformBuffers.size(); i++) {
        uniformBuffers[i].size = bufferSize;
        createBuffer(VkDev,
            uniformBuffers[i],
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        vkMapMemory(device,
            uniformBuffers[i].memory,
            0,
            bufferSize,
            NULL,
            &uniformBuffers[i].pointer);
    }
    return true;
}
RendererBase::~RendererBase() {
    vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
    }
    vkDestroyDescriptorPool(device, descriptorPool, nullptr);
    vkDestroyDescriptorSetLayout(device, descriptorSetLayout, nullptr);
    vkDestroyPipeline(device, graphicsPipeline, nullptr);
    for (auto& ubo : uniformBuffers) {
        vkUnmapMemory(device, ubo.memory);
        vkFreeMemory(device, ubo.memory, nullptr);
        vkDestroyBuffer(device, ubo.buffer, nullptr);
    }
}
void RendererBase::createDescriptorTools(const VulkanRenderDevice& VkDev, VkDescriptorImageInfo* imageInfo) {
    bool hasImageSampler = imageInfo != nullptr;
    std::vector<VkDescriptorPoolSize> poolSize(hasImageSampler ? 2 : 1);
    poolSize[0].descriptorCount = 1;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    if (hasImageSampler) {
        poolSize[1].descriptorCount = 1;
        poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.maxSets = VkDev.swapchainInfo.imageCount;
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes = poolSize.data();
    if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        std::cerr << "vkCreateDescriptorPool() - FAILED!";
        exit(EXIT_FAILURE);
    }
    std::vector<VkDescriptorSetLayoutBinding> bindings(hasImageSampler ? 2 : 1);
    bindings[0].binding = 0;
    bindings[0].descriptorCount = 1;
    bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    if (hasImageSampler) {
        bindings[1].binding = 1;
        bindings[1].descriptorCount = 1;
        bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[1].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    }
    VkDescriptorSetLayoutCreateInfo createInfoLayout{};
    createInfoLayout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    createInfoLayout.pBindings = bindings.data();
    createInfoLayout.bindingCount = bindings.size();
    if (vkCreateDescriptorSetLayout(device, &createInfoLayout, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
        std::cerr << "vkCreateDescriptorSetLayout() - FAILED!";
        exit(EXIT_FAILURE);
    }
    std::vector<VkDescriptorSetLayout> setLayouts(VkDev.swapchainInfo.imageCount, descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = descriptorPool;
    allocInfo.descriptorSetCount = setLayouts.size();
    allocInfo.pSetLayouts = setLayouts.data();
    descriptorSets.resize(setLayouts.size());
    if (vkAllocateDescriptorSets(device, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
        std::cerr << "vkAllocateDescriptorSets() - FAILED!";
        exit(EXIT_FAILURE);
    }
    for (uint32_t i = 0; i < descriptorSets.size(); i++) {
        VkDescriptorBufferInfo bufferInfo{};
        bufferInfo.buffer = uniformBuffers[i].buffer;
        bufferInfo.offset = 0;
        bufferInfo.range = VK_WHOLE_SIZE;

        std::vector<VkWriteDescriptorSet> descriptorWrite(hasImageSampler ? 2 : 1);
        descriptorWrite[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite[0].descriptorCount = 1;
        descriptorWrite[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite[0].dstArrayElement = 0;
        descriptorWrite[0].dstBinding = 0;
        descriptorWrite[0].dstSet = descriptorSets[i];
        descriptorWrite[0].pBufferInfo = &bufferInfo;
        if (hasImageSampler) {
            descriptorWrite[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrite[1].descriptorCount = 1;
            descriptorWrite[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrite[1].dstArrayElement = 0;
            descriptorWrite[1].dstBinding = 1;
            descriptorWrite[1].dstSet = descriptorSets[i];
            descriptorWrite[1].pImageInfo = imageInfo;
        }
        vkUpdateDescriptorSets(device, descriptorWrite.size(), descriptorWrite.data(), 0, 0);
    }
}
void RendererBase::createPipelineLayout(uint32_t pushConstantSize, VkShaderStageFlags pushConstantStage) {
    VkPipelineLayoutCreateInfo createInfo{};
    bool hasPushConstant = pushConstantSize != 0;
    if (hasPushConstant) {
        VkPushConstantRange range{};
        range.offset = 0;
        range.size = pushConstantSize;
        range.stageFlags = pushConstantStage;
        createInfo.pPushConstantRanges = &range;
        createInfo.pushConstantRangeCount = 1;
    }
    createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    createInfo.setLayoutCount = 1;
    createInfo.pSetLayouts = &descriptorSetLayout;
    if (vkCreatePipelineLayout(device, &createInfo, NULL, &pipelineLayout) != VK_SUCCESS) {
        std::cerr << "vkCreatePipelineLayout() - FAILED!";
        exit(EXIT_FAILURE);
    }
}