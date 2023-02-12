#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_ENABLE_EXPERIMENTAL
#define GLFW_INCLUDE_VULKAN
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/hash.hpp>
#include <GLFW/glfw3.h>

#include <array>
#include <cstring>
#include <unordered_map>
#include <set>
#include <chrono>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

#include "shaderloader.h"


const uint32_t WINDOW_WIDTH = 1200;
const uint32_t WINDOW_HEIGHT = 900;

const std::string MODEL_PATH = "models/viking_room.obj";
const std::string TEXTURE_PATH = "textures/viking_room.png";

const uint32_t MAX_FRAMES_IN_FLIGHT = 2;

const std::array<const char*, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation",
    };

const std::array<const char*, 1> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

static VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) {
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

static void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger,
    const VkAllocationCallbacks* pAllocator) {
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        func(instance, debugMessenger, pAllocator);
    }
}

//列族配置
struct QueueFamilyIndices {
    uint32_t graphicsFamilyIndex;
    uint32_t computeFamilyIndex;
    uint32_t presentFamilyIndex;
};

//交换链配置
struct SwapchainConfiguration {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    std::vector<VkPresentModeKHR> presentModes;
};

//顶点结构
struct Vertex {
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;

    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};

        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, color);

        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }

    bool operator==(const Vertex& other) const {
        return pos == other.pos && color == other.color && texCoord == other.texCoord;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.color) << 1)) >> 1) ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

//alignas
struct UniformBufferObject {
    alignas(16) glm::mat4 model;
    alignas(16) glm::mat4 view;
    alignas(16) glm::mat4 proj;
};

class Application {
public:
    Application();

    ~Application();

    void run();

private:

    GLFWwindow* window;

    VkInstance instance;
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
    VkSampleCountFlagBits msaaSamples = VK_SAMPLE_COUNT_1_BIT;
    VkDevice logicalDevice;

    VkQueue graphicsQueue;
    VkQueue presentQueue;

    VkSurfaceKHR surface;

    VkSwapchainKHR swapchain;
    std::vector<VkImage> swapchainImages;
    std::vector<VkImageView> swapchainImageViews;
    VkExtent2D swapchainExtent;
    VkFormat swapchainImageFormat;
    std::vector<VkFramebuffer> swapchainFramebuffers;
    std::vector<VkFramebuffer> uiFramebuffers;

    VkRenderPass renderPass;
    VkRenderPass uiRenderPass;

    VkDescriptorSetLayout descriptorSetLayout;

    VkPipeline graphicsPipeline;
    VkPipelineLayout graphicsPipelineLayout;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    std::vector<VkBuffer> uniformBuffers;
    std::vector<VkDeviceMemory> uniformBuffersMemory;
    std::vector<void*> uniformBuffersMapped;

    VkCommandPool commandPool;
    VkCommandPool uiCommandPool;
    std::vector<VkCommandBuffer> commandBuffers;
    std::vector<VkCommandBuffer> uiCommandBuffers;

    uint32_t imageCount = 0;
    uint32_t currentFrame = 0;
    
    std::vector<VkSemaphore> imageAvailableSemaphores;
    std::vector<VkSemaphore> renderFinishedSemaphores;
    std::vector<VkFence> inFlightFences;
    std::vector<VkFence> imagesInFlight;

    VkDescriptorPool descriptorPool;
    std::vector<VkDescriptorSet> descriptorSets;

    VkDescriptorPool uiDescriptorPool;

    uint32_t mipLevels;
    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkImageView textureImageView;
    VkSampler textureSampler;

    VkImage depthImage;
    VkDeviceMemory depthImageMemory;
    VkImageView depthImageView;


    VkImage colorImage;
    VkDeviceMemory colorImageMemory;
    VkImageView colorImageView;

    bool framebufferResized = false;

    std::vector<const char*> requiredExtensions;

    QueueFamilyIndices queueIndices;

    //debug信使
    VkDebugUtilsMessengerEXT debugMessenger;

    //debug回调
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData) {

        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;

        return VK_FALSE;
    }

    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

    static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
        auto app = reinterpret_cast<Application*>(glfwGetWindowUserPointer(window));
        app->framebufferResized = true;
    }

    static void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo) {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    VkCommandBuffer beginSingleTimeCommands(VkCommandPool cmdPool);

    bool checkDeviceExtensions(VkPhysicalDevice device);

    bool checkValidationLayerSupport();

    void cleanupSwapchain();

    void cleanupUIResources();

    void createCommandBuffers();

    //创建顶点缓冲区
    void createVertexBuffer();

    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    void createIndexBuffer();
    
    void createCommandPool();

    void createDescriptorPool();

    void createFramebuffers();

    void createGraphicsPipeline();

    void createImageViews();

    void createInstance();

    void createLogicalDevice();

    void createRenderPass();

    void createUICommandBuffers();

    void createUICommandPool(VkCommandPool *commandPool, VkCommandPoolCreateFlags flags);

    void createUIDescriptorPool();

    void createUIFramebuffers();

    void createUIRenderPass();

    VkShaderModule createShaderModule(const std::vector<char> &shaderCode);

    void createSurface();

    void createSwapchain();

    void createSyncObjects();

    void drawFrame();

    static void drawUI();

    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkCommandPool cmdPool);

    void getDeviceQueueIndices();

    std::vector<const char*> getRequiredExtensions() const;

    void initUI();

    void initVulkan();

    void initWindow();

    bool isDeviceSuitable(VkPhysicalDevice device);

    void pickPhysicalDevice();

    VkExtent2D pickSwapchainExtent(const VkSurfaceCapabilitiesKHR &surfaceCapabilities);

    static VkPresentModeKHR pickSwapchainPresentMode(const std::vector<VkPresentModeKHR> &presentModes);

    static VkSurfaceFormatKHR pickSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);

    SwapchainConfiguration querySwapchainSupport(const VkPhysicalDevice &physicalDevice);

    void recordUICommands(uint32_t bufferIdx);

    void recreateSwapchain();

    void setupDebugMessenger();

    void createDescriptorSetLayout();

    void createUniformBuffers();

    void updateUniformBuffer(uint32_t currentImage);

    void createDescriptorSets();

    void createTextureImage();

    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits numSamples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
    
    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    
    void createTextureImageView();

    VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

    void createTextureSampler();

    void createDepthResources();

    VkFormat findDepthFormat();

    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

    bool hasStencilComponent(VkFormat format);

    void loadModel();

    void generateMipmaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    
    void createColorResources();

    VkSampleCountFlagBits getMaxUsableSampleCount();
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }
};