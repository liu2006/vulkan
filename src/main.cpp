#include "application.hpp"

int main()
{
    try {
        std::unique_ptr<Application> app = std::make_unique<Application>(1200, 1000, "liu");
        app->run();
    } catch (const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}
