module;
#include <vector>
#include <vulkan/vulkan_raii.hpp>
#include <iostream>
#include <SDL3/SDL_vulkan.h>
module Application:internal;
import :validationLayers;

vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    assert(!availableFormats.empty());
    auto formatIter{std::ranges::find_if(availableFormats, [](const auto &format) {
        return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    })};
    return (formatIter != availableFormats.end()) ? *formatIter : availableFormats[0];
}

vk::PresentModeKHR chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &presentModes)
{
    assert(std::ranges::any_of(presentModes,
                               [](const auto &presentMode) { return presentMode == vk::PresentModeKHR::eFifo; }));
    return std::ranges::any_of(presentModes,
                               [](const auto &presentMode) { return presentMode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}

vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
        return capabilities.currentExtent;
    int width{}, height{};
    SDL_GetWindowSizeInPixels(window, &width, &height);
    return {std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
            std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)};
}

uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    auto minImageCount{std::max(3u, capabilities.minImageCount)};
    if ((capabilities.maxImageCount > 0) && (capabilities.maxImageCount < minImageCount))
        minImageCount = capabilities.maxImageCount;
    return minImageCount;
}


std::vector<const char *> getRequiredExtensions()
{
    uint32_t sdlExtensionCount{};
    auto sdlExtensions{SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount)};
    std::vector<const char *> extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);
    if (enableValidationLayers)
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    return extensions;
}

VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                               vk::DebugUtilsMessageTypeFlagsEXT type,
                                               const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cerr << std::format("validation layer type: {}, message: {}", to_string(type), pCallbackData->pMessage);
    }
    return vk::False;
}

const std::vector<const char *> requiredDeviceExtensions{vk::KHRSwapchainExtensionName};

