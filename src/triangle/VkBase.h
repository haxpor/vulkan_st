#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <optional>
#include <string>
#include <vector>

// define to enable validation layers
#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

// get access to extension functions
static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class VkBase {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        inline bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

public:
    void init(const int width, const int height, std::string title);
    void run();

private:
    void initWindow(const int width, const int height, std::string title);
    void initVulkan();
    void mainLoop();
    void drawFrame();
    void cleanup();
    void createInstance();
    std::vector<const char*> getRequiredExtensions() const;
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData);
    static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
    void setupDebugMessenger();
    void createSyncObjects();
    void createCommandPool();
    void createVertexBuffer();
    void createCommandBuffers();
    void createFramebuffers();
    void createRenderPass();
    VkShaderModule createShaderModule(const std::vector<char>& code) const;
    void createGraphicsPipeline();
    void createImageViews();
    void createSwapChain();
    void createSurface();
    void createLogicalDevice();
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) const;
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) const;
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) const;
    void pickPhysicalDevice();
    int rateDevice(VkPhysicalDevice device, bool& isDiscrete) const;
    void printDeviceInfo(VkPhysicalDevice& device) const;
    std::string getDriverVersionString(uint32_t vendorID, uint32_t driverVersion) const;
    std::string getVendorString(uint32_t vendorID) const;
    std::string getDeviceTypeString(uint32_t deviceType) const;
    bool isDeviceSuitable(VkPhysicalDevice device) const;
    bool checkDeviceExtensionSupport(VkPhysicalDevice device) const;
    bool checkAllRequiredExtensionsSupported() const;
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) const;
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) const;
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) const;
    bool checkValidationLayerSupport() const;
    void recreateSwapChain();
    void cleanupSwapChain();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void createIndexBuffer();
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void createDescriptorSetLayout();
    void createUniformBuffers();
    void updateUniformBuffer(uint32_t currentImage);
    void createDescriptorPool();
    void createDescriptorSets();

private:
    GLFWwindow* window;
    VkInstance instance;
    VkDebugUtilsMessengerEXT debugMessenger;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkDevice device;
    VkQueue graphicsQueue;
    VkQueue presentQueue;
    VkSurfaceKHR surface;
    VkSwapchainKHR swapChain;
    std::vector<VkImage> swapChainImages;
    VkFormat swapChainImageFormat;
    VkExtent2D swapChainExtent;
    std::vector<VkImageView> swapChainImageViews;
    VkRenderPass renderPass;
    VkDescriptorSetLayout descriptorSetLayout;
    VkPipelineLayout pipelineLayout;
    VkPipeline graphicsPipeline;
    std::vector<VkFramebuffer> swapChainFramebuffers;
    VkCommandPool commandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> imagesInFlight;
    QueueFamilyIndices queueFamilyIndices;
    size_t currentFrame = 0;
    size_t semaphoreIndex = 0;
    std::string windowTitle;
    bool framebufferResized = false;
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;
    std::vector<VkBuffer> uniformBuffers;       // uniform buffer for each swapchain's image
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    uint32_t numRenderedFrames = 0;
    float fps = 0.0f;
    double prevTime = 0.0f;
};
