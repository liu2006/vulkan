#include "sdlException.hpp"
#include "application.hpp"


Application::Application(uint32_t width, uint32_t height, const char *prefix)
    : windowWidth{width}, windowHeight{height}, title{prefix}
{
}

Application::~Application() {}

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}
void Application::initWindow()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw SdlException("Couldn't initialize SDL");
    if (!SDL_Vulkan_LoadLibrary(nullptr))
        throw SdlException("Couln't load vulkan library");
    window = SDL_CreateWindow(title, windowWidth, windowHeight, SDL_WINDOW_VULKAN | SDL_WINDOW_HIDDEN);
    renderer = SDL_CreateRenderer(window, nullptr);
}

void Application::initVulkan() {}
void Application::mainLoop()
{
    SDL_ShowWindow(window);
    while (running) {
        SDL_Event event{};
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) {
                running = false;
            }
        }
    }
}
void Application::cleanup()
{
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
}

