#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>

constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{600};
constexpr const char *TITLE{"vulkan"};

class Application
{
private:
    uint32_t windowWidth{};
    uint32_t windowHeight{};
    const char *title{};
    bool running{true};
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
public:
    Application();
    ~Application();
    void run();
};
