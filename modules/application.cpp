module;
#include <iostream>
#include <memory>
#include <vector>
#include <cstdint>
#include <vulkan/vulkan_raii.hpp>
#include <SDL3/SDL.h>
export module app;

export class Application
{
private:
    struct SDLGuard
    {
        SDLGuard() { SDL_Init(SDL_INIT_VIDEO); }
        ~SDLGuard()
        {
            SDL_Quit();
            std::cout << "Destroyed SDL\n";
        }
    };
    SDLGuard sdlGuard;
    std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> window{nullptr, SDL_DestroyWindow};
    std::uint32_t width;
    std::uint32_t height;
    const char *title;
    bool running{true};
    vk::raii::Context context;
    vk::raii::Instance instance{nullptr};
    vk::raii::DebugUtilsMessengerEXT debugMessenger{nullptr};
    vk::raii::SurfaceKHR surface{nullptr};
    
    vk::raii::PhysicalDevice physicalDevice{nullptr};
    vk::raii::Device device{nullptr};
    vk::raii::Queue queue{nullptr};
    vk::raii::SwapchainKHR swapChain{nullptr};
    std::vector<vk::Image> swapChainImages{};
    std::vector<vk::raii::ImageView> swapChainImageViews{};
    
    vk::SurfaceFormatKHR swapChainSurfaceFormat;
    vk::Extent2D swapChainExtent;
    
    void initWindow();
    void initVulkan();
    void mainLoop();
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicDevice();
    void createSwapChain();
    void createImageViews();
    void createGraphicsPipeline();
    
public:
    ~Application();
    Application() : width{800}, height{600}, title{"vulkan"} {}
    Application(std::uint32_t width, std::uint32_t height, const char *title);
    void run();
};

