#pragma once
#include <format>
#include <stdexcept>
#include <SDL3/SDL.h>

class SdlException final : public std::runtime_error
{
public:
    explicit SdlException(const std::string &msg);
};

