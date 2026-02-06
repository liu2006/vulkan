#include "application.hpp"


Application::Application(uint32_t width, uint32_t height, const char *prefix)
    : windowWidth{width}, windowHeight{height}, title{prefix}
{
}

Application::~Application() {}

void Application::run() {}
