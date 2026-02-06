#pragma once
#include <SDL3/SDL.h>
#include <exception>
#include <cstdlib>
#include <SDL3/SDL_vulkan.h>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <cstdint>

constexpr uint32_t WIDTH{800};
constexpr uint32_t HEIGHT{600};
constexpr const char *TITLE{"vulkan"};

class Application
{
private:
    SDL_Window *window{nullptr};
    SDL_Renderer *renderer{nullptr};
    uint32_t windowWidth{};
    uint32_t windowHeight{};
    const char *title{};
    bool running{true};
    void initWindow();
    void initVulkan();
    void mainLoop();
    void cleanup();
public:
    Application(uint32_t width = WIDTH, uint32_t height = HEIGHT, const char *prefix = TITLE);
    ~Application();
    void run();
};
