#include "sdlException.hpp"

SdlException::SdlException(const std::string &msg)
    : std::runtime_error{std::format("{}: {}", msg, SDL_GetError())}
{
    
}
