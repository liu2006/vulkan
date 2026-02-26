#include <memory>
#include <cstdlib>
#include <iostream>
#include <SDL3/SDL.h>
import Application;

int main()
{
    try {
        std::unique_ptr<Application> app{std::make_unique<Application>()};
        app->run();
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }
    SDL_Quit();
    return EXIT_SUCCESS;
}
