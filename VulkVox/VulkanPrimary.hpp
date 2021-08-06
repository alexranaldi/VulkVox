#ifndef VULKAN_PRIMARY
#define VULKAN_PRIMARY

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <string>
#include <optional>
#include <set>
#include <fstream>
#include <array>
#include <chrono>
#include <memory>
#include <queue>

#include <glm/glm.hpp>
#define GLM_FORCE_RADIANS
//#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Vertex.hpp"
#include "Camera.hpp"

struct WorldObjects;

class VulkanBootStrapper;

class VulkanPrimary {
public:
    VulkanPrimary();

    void drawFrame();

    void bootstrap();

    void recreateSwapChain();

    void cleanup();

    void updateUniformBuffer(uint32_t currentImage);

    VkDevice& getDevice();

    VkInstance& getInstance();

    VkDescriptorSetLayout& getDescriptorSetLayout();

    void updateSecondaryCommandBuffers(VkFramebuffer& frameBuffer, VkCommandBuffer& primaryCommandBuffer, VkCommandBuffer& secondaryCommandBuffer, VkCommandBufferInheritanceInfo inheritanceInfo) ;

    void setWorldObjects(WorldObjects* in);

    void updateCommandBuffers(size_t index);

    VkCommandBuffer getSecondaryCommandBuffer();
    void createSecondaryCommandBuffer();

    std::unique_ptr<VulkanBootStrapper> mBootStrapper;

    VkQueue presentQueue;
    VkQueue graphicsQueue;
    VkDevice mDevice;
    GLFWwindow* window;
    VkInstance mInstance;
    VkDebugUtilsMessengerEXT debugMessenger;

    VkCommandPool primaryCommandPool;
    VkCommandPool objectCommandPool;

    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSurfaceKHR surface;

    const uint32_t WIDTH = 800;
    const uint32_t HEIGHT = 600;

    const std::vector<const char*> deviceExtensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;

    std::vector<VkImageView> swapChainImageViews;

    VkPipeline graphicsPipeline;

    VkRenderPass renderPass;
    VkDescriptorSetLayout mDescriptorSetLayout;
    VkPipelineLayout pipelineLayout;

    std::vector<VkFramebuffer> swapChainFramebuffers;

    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> objectCommandBuffers;

    std::queue<VkCommandBuffer> availableSecondaryCommandBuffers;

    VkSemaphore imageAvailableSemaphore;
    VkSemaphore renderFinishedSemaphore;

    const int MAX_FRAMES_IN_FLIGHT = 2;

    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;
    size_t currentFrame = 0;
    bool framebufferResized = false;

    std::vector<Vertex> cubeVertices;
    std::vector<VertexIndex> cubeVertexIndices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    VkBuffer indexStagingBuffer;
    VkDeviceMemory indexStagingBufferMemory;
    VkBuffer vertexStagingBuffer;
    VkDeviceMemory vertexStagingBufferMemory;

    VkDeviceSize vertexBufferSize;
    VkDeviceSize indexBufferSize;

    VkDeviceSize vertexStagingBufferSize;
    VkDeviceSize indexStagingBufferSize;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;

    VkMemoryRequirements memRequirements;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;

    WorldObjects* wo;

    Camera camera;

#ifdef NDEBUG
    const bool enableValidationLayers = false;
#else
    const bool enableValidationLayers = true;
#endif

};

#endif