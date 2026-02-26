module;
#include <memory>
#include <SDL3/SDL.h>
#include <SDL3/SDL_vulkan.h>
#include <cstdint>
#include <iostream>
module Application;

Application::Application(uint32_t width, uint32_t height, const char *title)
    : width{width}, height{height}, title{title}
{
}

Application::~Application()
{
    std::cout << "Destroyed Application\n";
}

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
}

void Application::initWindow()
{
    SDL_Init(SDL_INIT_VIDEO);
    window = SDL_CreateWindow(title, width, height, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
}

void Application::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicDevice();
    createSwapChain();
    createImageViews();
}
void Application::mainLoop()
{
    SDL_ShowWindow(window);
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT)
                running = false;
        }
    }
}
                          

