#pragma once
#include <limits>
#include <vector>
#include <SDL3/SDL.h>
#include <cstdint>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <memory>
#include <algorithm>
#include <cstring>

const std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers{false};
#else
constexpr bool enableValidationLayers{true};
#endif

class Application
{
private:
    uint32_t width;
    uint32_t height;
    const char *title;
    bool running{true};
    SDL_Window *window{nullptr};
    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::SwapchainKHR swapChain{nullptr};
    std::vector<vk::Image> swapChainImages;
    
    
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();
    std::vector<const char *> getRequiredExtensions();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicDevice();
    void createSwapChain();
    std::vector<const char *> requiredDeviceExtensions{vk::KHRSwapchainExtensionName};

public:
    Application();
    Application(uint32_t width, uint32_t height, const char *title);
    ~Application();
    void run();
};
