#include "ImGuiRenderer.h"
#include "Camera.h"
#define IMGUI_IMPLEMENTATION
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>



ImGuiRenderer::ImGuiRenderer(VulkanInstance& VkInst,const VulkanRenderDevice& VkDev, GLFWwindow* window) : RendererBase(VkDev) {
    pVkInst = &VkInst;
    createDescriptorTools(VkDev);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    if (!ImGui_ImplGlfw_InitForVulkan(window, true)) {
        std::cerr << "FAILED TO INIT IMGUI + GLFW!";
        exit(EXIT_FAILURE);
    }
    recreateSwapchainComponents(VkDev);
}
ImGuiRenderer::~ImGuiRenderer() {
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void ImGuiRenderer::fillCommandBuffer(VkCommandBuffer commandBuffer, uint32_t currentImage) {
    bool isOpened = true;
   // auto pos = camera.getPos();
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    auto& style = ImGui::GetStyle();
    ImGui::Begin("123", &isOpened, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    ImGui::SetWindowPos(ImVec2(0.0f, 0.0f));
    ImGui::Text("HELLO MY NIGGA!");
    //ImGui::Text("FPS: %d", static_cast<int>(1000.f / deltaTime));
    //ImGui::Text("Total vertices: %i", vertices.size());
    //ImGui::Text("Camera position:\nX = %.2f \nY = %.2f% \nZ = %.2f", pos.x, pos.y, pos.y);
    //ImGui::DragFloat("Scroll speed", &camera.scrollSpeed, 1.0f, 0.1f, 100.f, "%3.2f", ImGuiSliderFlags_AlwaysClamp);
    //ImGui::DragFloat3("Model size: ", , 0.01f, 0.00001f, 10000.f, "%5.5f", ImGuiSliderFlags_AlwaysClamp);
    //ImGui::Checkbox("Wireframe", &isWireframeShown);
    ImGui::End();
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}
void ImGuiRenderer::createDescriptorTools(const VulkanRenderDevice& VkDev) {
    std::vector<VkDescriptorPoolSize> poolSize(2);
    poolSize[0].descriptorCount = VkDev.swapchainInfo.imageCount;
    poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize[1].descriptorCount = VkDev.swapchainInfo.imageCount;
    poolSize[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    VkDescriptorPoolCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    createInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    createInfo.maxSets = VkDev.swapchainInfo.imageCount;
    for (auto& i : poolSize) {
        createInfo.maxSets += i.descriptorCount;
    }
    createInfo.poolSizeCount = poolSize.size();
    createInfo.pPoolSizes = poolSize.data();
    if (vkCreateDescriptorPool(device, &createInfo, nullptr, &descriptorPool) != VK_SUCCESS) {
        std::cerr << "vkCreateDescriptorPool() - FAILED!";
        exit(EXIT_FAILURE);
    }
}
void ImGuiRenderer::cleanupSwapchainComponents() {

}
void ImGuiRenderer::recreateSwapchainComponents(const VulkanRenderDevice& VkDev) {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(VkDev.physicalDevice, pVkInst->surface, &surfaceCapabilities);
    ImGui_ImplVulkan_InitInfo info{};
    info.RenderPass = VkDev.renderPass;
    info.PhysicalDevice = VkDev.physicalDevice;
    info.Device = VkDev.device;
    info.ImageCount = VkDev.swapchainInfo.imageCount;
    info.Instance = pVkInst->instance;
    info.DescriptorPool = descriptorPool;
    info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    info.MinImageCount = surfaceCapabilities.minImageCount;
    info.Queue = VkDev.graphicsQueue;
    if (!ImGui_ImplVulkan_Init(&info)) {
        std::cerr << "FAILED TO INIT IMGUI + VULKAN!";
        exit(EXIT_FAILURE);
    }
}