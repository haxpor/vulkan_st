#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <cstring>
#include <map>
#include <queue>
#include <tuple>
#include <optional>
#include <string>
#include <set>
#include <cstdint>
#include <algorithm>

const int WIDTH = 800;
const int HEIGHT = 600;

const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// define to enable validation layers
#ifndef NDEBUG
#define ENABLE_VALIDATION_LAYERS
#endif

// wrapper function
// it get function address of vkCreateDebugUtilsMessenger and calls it internally
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

class HelloTriangleApplication {
public:
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

public:
    void run() {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device) {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        int i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
            }

            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
                indices.presentFamily = i;

            if (indices.isComplete())
                break;

            ++i;
        }

        return indices;
    }

    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        // query for basic capabilities
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

        // query for formats
        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
        }

        // query for presentation mode
        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
        }
        return details;
    }

    void pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
        if (deviceCount == 0)
            throw std::runtime_error("failed to find GPUs with Vulkan support!");

        std::vector<VkPhysicalDevice> devices (deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // device scores
        typedef int DeviceScore;
        typedef bool DeviceIsDiscreteGPU;
        std::priority_queue<std::tuple<VkPhysicalDevice, DeviceScore, DeviceIsDiscreteGPU>> pq;

        for (int i=0; i<devices.size(); ++i) {
            bool isDiscrete;
            int score = rateDevice(devices[i], isDiscrete);
            if (score > 0)
                pq.push( std::make_tuple(devices[i], score, isDiscrete) );
        }

        if (pq.size() > 0) {
            // custom selection of gpu can be added here
            physicalDevice = std::get<0>(pq.top());        // for now, accept whether it's discrete GPU or others
        }

        if (physicalDevice == VK_NULL_HANDLE)
            throw std::runtime_error("failed to find a suitable GPU with vulkan support!");

        printDeviceInfo(physicalDevice);
    }

    ///
    /// Rate the input device then return score.
    int rateDevice(VkPhysicalDevice device, bool& isDiscrete) {
        VkPhysicalDeviceProperties deviceProperties;
        vkGetPhysicalDeviceProperties(device, &deviceProperties);

        VkPhysicalDeviceFeatures deviceFeatures;
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        // return early if not suitable to be used
        if (!isDeviceSuitable(device, deviceProperties, deviceFeatures))
            return 0;

        int score = 0;

        if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            score += 100;
            isDiscrete = true;
        }
        else {
            score += 30;
            isDiscrete = false;
        }

        return score;
    }

    ///
    /// Print device's information
    void printDeviceInfo(VkPhysicalDevice& device) {
        VkPhysicalDeviceProperties2 deviceProps2;
        VkPhysicalDeviceDriverProperties driverProps;
        driverProps.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DRIVER_PROPERTIES;
        driverProps.pNext = nullptr;

        deviceProps2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        deviceProps2.pNext = &driverProps;
        vkGetPhysicalDeviceProperties2(device, &deviceProps2);

        VkPhysicalDeviceProperties deviceProps = deviceProps2.properties;

        std::cout << "Device Information\n";
        std::cout << "  API version: " << VK_VERSION_MAJOR(deviceProps.apiVersion) << '.' << VK_VERSION_MINOR(deviceProps.apiVersion) << '.' << VK_VERSION_PATCH(deviceProps.apiVersion) << '\n';

        // driverVersion is vendor-specific
        std::cout << "  Driver version: " << getDriverVersionString(deviceProps.vendorID, deviceProps.driverVersion) << '\n';
        std::cout << "  Vendor ID: " << std::hex << deviceProps.vendorID << " [" << getVendorString(deviceProps.vendorID) << "]\n";
        std::cout << "  Device type: " << getDeviceTypeString(deviceProps.deviceType) << '\n';
        std::cout << "  Device name: " << deviceProps.deviceName << '\n';

        std::cout << "  Driver version (string): " << driverProps.driverInfo << '\n';
        std::cout << "  Driver name: " << driverProps.driverName << '\n';
        std::cout << "  Driver info: " << driverProps.driverInfo << '\n';
        std::cout << "  Driver ID: " << std::hex << driverProps.driverID << '\n';
    }

    std::string getDriverVersionString(uint32_t vendorID, uint32_t driverVersion) const {
        switch (vendorID) {
            // AMD
            case 0x1002:
                return std::to_string( VK_VERSION_MAJOR(driverVersion) ) + "." + std::to_string( VK_VERSION_MINOR(driverVersion) ) + "." + std::to_string( VK_VERSION_PATCH(driverVersion) );
            // NVIDIA
            case 0x10DE:
                return std::to_string( (driverVersion >> 22) & 0x3ff ) + "." + std::to_string( (driverVersion >> 14) & 0x0ff ) + "." + std::to_string( (driverVersion >> 6) & 0x0ff ) + "." + std::to_string( driverVersion & 0x003f );
            // Intel
            case 0x8086:
                return std::to_string( (driverVersion >> 14) ) + "." + std::to_string( driverVersion & 0x3fff );

            // Use vulkan convention for the less
            default:
                return std::to_string( VK_VERSION_MAJOR(driverVersion) ) + "." + std::to_string( VK_VERSION_MINOR(driverVersion) ) + "." + std::to_string( VK_VERSION_PATCH(driverVersion) );
        }
    }

    std::string getVendorString(uint32_t vendorID) const {
        switch (vendorID) {
            case 0x1002: return "AMD";
            case 0x1010: return "ImgTec";
            case 0x10DE: return "NVIDIA";
            case 0x13B5: return "ARM";
            case 0x5143: return "Qualcomm";
            case 0x8086: return "Intel";
            default: return "Unknown";
        }
    }

    std::string getDeviceTypeString(uint32_t deviceType) const {
        switch (deviceType) {
            case VK_PHYSICAL_DEVICE_TYPE_OTHER:
                return "Other";
            case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
                return "iGPU";
            case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
                return "Discrete GPU";
            case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
                return "Virtual GPU";
            case VK_PHYSICAL_DEVICE_TYPE_CPU:
                return "CPU";
            default:
                return "Unknown";
        }
    }

    bool isDeviceSuitable(VkPhysicalDevice device, VkPhysicalDeviceProperties& prop, VkPhysicalDeviceFeatures& features) {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);
        bool swapChainAdequate = false;
        if (extensionsSupported) {
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
        }
        return indices.isComplete() && extensionsSupported && swapChainAdequate;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device) {
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        // remove to avoid O(N^2) but the size is very small, so should be very similar
        for (const auto& extension : availableExtensions) {
            requiredExtensions.erase(extension.extensionName);
        }

        // if empty then all extensions are supported
        return requiredExtensions.empty();
    }

    bool checkAllRequiredExtensionsSupported() {
        // get all extensions required by glfw
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        // get all extensions available from vulkan
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions (extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
// O(N^2) checking
        for (int i=0; i<glfwExtensionCount; ++i) {
            bool thisExtFound = false;
            for (int j=0; j<extensionCount; ++j) {
                if (std::strcmp(glfwExtensions[i], extensions[j].extensionName) == 0) {
                    thisExtFound = true;
                    break;
                }
            }
            if (!thisExtFound) {
                std::cerr << glfwExtensions[i] << " is not supported\n";
                return false;
            }
        }

        return true;
    }

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentMode) {
        for (const auto& availablePresentMode : availablePresentMode) {
            if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
                return availablePresentMode;
            }
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
        if (capabilities.currentExtent.width != UINT32_MAX) {
            return capabilities.currentExtent;
        }
        else {
            VkExtent2D actualExtent = {WIDTH, HEIGHT};

            actualExtent.width = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
            actualExtent.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

            return actualExtent;
        }
    }

    bool checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // O(N^2) checking
        for (const char* layerName : validationLayers) {
            bool thisLayerFound = false;

            for (const auto& layerProperties : availableLayers) {
                if (std::strcmp(layerName, layerProperties.layerName) == 0) {
                    thisLayerFound = true;
                    break;
                }
            }
            if (!thisLayerFound) {
                std::cerr << "layer " << layerName << " is not supported\n";
                return false;
            }
        }

        return true;
    }

private:
    void initWindow() {
        glfwInit();
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }

    void initVulkan() {
        createInstance();
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createSwapChain();
    }

    void createSwapChain() {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
        VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

        uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
        if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
            imageCount = swapChainSupport.capabilities.maxImageCount;
        }

        VkSwapchainCreateInfoKHR createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface = surface;
        createInfo.minImageCount = imageCount;
        createInfo.imageFormat = surfaceFormat.format;
        createInfo.imageColorSpace = surfaceFormat.colorSpace;
        createInfo.imageExtent = extent;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily) {
            createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices = queueFamilyIndices;
        }
        else {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            createInfo.queueFamilyIndexCount = 0;
            createInfo.pQueueFamilyIndices = nullptr;
        }
        createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode = presentMode;
        createInfo.clipped = VK_TRUE;
        createInfo.oldSwapchain = VK_NULL_HANDLE;

        if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain) != VK_SUCCESS) {
            throw std::runtime_error("failed to create swap chain!");
        }

        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
        swapChainImages.resize(imageCount);
        vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapChainImages.data());

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent = extent;
    }

    void createSurface() {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS) {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    void createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        VkPhysicalDeviceFeatures deviceFeatures = {};

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();
        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
#else
        createInfo.enabledLayerCount = 0;
#endif

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS) {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    void setupDebugMessenger() {
#ifdef ENABLE_VALIDATION_LAYERS
        VkDebugUtilsMessengerCreateInfoEXT createInfo;
        populateDebugMessengerCreateInfo(createInfo);

        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS) {
            throw std::runtime_error("failed to set up debug messenger!");
        }
#endif
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    void mainLoop() {
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();
        }
    }

    void cleanup() {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        vkDestroyDevice(device, nullptr);
#ifdef ENABLE_VALIDATION_LAYERS
        DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
#endif

        vkDestroySurfaceKHR(instance, surface, nullptr);
        vkDestroyInstance(instance, nullptr);
        glfwDestroyWindow(window);
        glfwTerminate();
    }

    void createInstance() {
#ifdef ENABLE_VALIDATION_LAYERS
        if (!checkValidationLayerSupport()) {
            throw std::runtime_error("validation layers requested, but not available!");
        }
#endif

        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Hello Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_2;    // my GPU supports (for now) until 1.1.114

        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo;

        auto extensions = getRequiredExtensions();
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
#ifdef ENABLE_VALIDATION_LAYERS
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
        populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
#else
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
#endif

        if (vkCreateInstance(&createInfo, nullptr, &instance) != VK_SUCCESS) {
            throw std::runtime_error("failed to create instance!");
        }
    }

    std::vector<const char*> getRequiredExtensions() {
        uint32_t glfwExtensionCount = 0;
        const char** glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char*> extensions;
        for (int i=0; i<glfwExtensionCount; ++i) {
            extensions.push_back(glfwExtensions[i]);
        }
#ifdef ENABLE_VALIDATION_LAYERS
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

        return extensions;
    }

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    }

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
};

int main() {
    HelloTriangleApplication app;

    try {
        app.run();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
