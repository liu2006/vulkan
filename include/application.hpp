#pragma once
#include <algorithm>
#include <SDL3/SDL.h>
#include <exception>
#include <cstdlib>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <cstdint>
#include <vector>
#include <cstring>

constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{600};
constexpr const char *TITLE{"vulkan"};
const std::vector<const char *> validationLayers{"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers{false};
#else
constexpr bool enableValidationLayers{true};
#endif

class Application
{
private:
    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};
    uint32_t windowWidth{};
    uint32_t windowHeight{};
    const char *title{};
    bool running{true};
    vk::raii::Context context{};
    vk::raii::Instance instance{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
    void createInstance();
    void setupDebugMessenger();
    void pickPhysicalDevice();
    void createLogicDevice();
    void createSurface();
    std::vector<const char *> getRequiredExtensions();
    std::vector<const char *> requiredDeviceExtensions{vk::KHRSwapchainExtensionName};
public:
    Application(uint32_t width = WIDTH, uint32_t height = HEIGHT, const char *prefix = TITLE);
    ~Application();
    void run();
};
