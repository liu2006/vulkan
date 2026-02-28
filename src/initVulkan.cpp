module;
#include <limits>
#include <SDL3/SDL_vulkan.h>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <format>
#include <stdexcept>
#include <vector>
#include <vulkan/vulkan_raii.hpp>
module Application;
import :internal;

void Application::createInstance()
{
    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = vk::ApiVersion14;

    std::vector<const char *> requiredLayers{};
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }
    auto layerProperties{context.enumerateInstanceLayerProperties()};

    auto unsupportedLayerIter{std::ranges::find_if(requiredLayers, [&layerProperties](const auto &requiredLayer) {
        return std::ranges::none_of(layerProperties, [requiredLayer](const auto &layerProperty) {
            return std::strcmp(layerProperty.layerName, requiredLayer) == 0;
        });
    })};
    if (unsupportedLayerIter != requiredLayers.end())
        throw std::runtime_error(std::format("required laqyer not supported: {}", *unsupportedLayerIter)); 

    auto requiredExtensions{getRequiredExtensions()};
    auto extensionProperties{context.enumerateInstanceExtensionProperties()};

    auto unsupportedExtensionIter{
        std::ranges::find_if(requiredExtensions, [&extensionProperties](const auto &requiredExtension) {
            return (std::ranges::none_of(extensionProperties, [requiredExtension](const auto &extensionProperty) {
                return std::strcmp(extensionProperty.extensionName, requiredExtension) == 0;
            }));
        })};
    if (unsupportedExtensionIter != requiredExtensions.end())
            throw std::runtime_error(std::format("required extension not supported: {}", *unsupportedExtensionIter));

    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledLayerCount = requiredLayers.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledExtensionCount = requiredExtensions.size();

    instance = vk::raii::Instance(context, createInfo);
}

void Application::setupDebugMessenger()
{
    if (!enableValidationLayers)
        return;
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning};
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance};
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
    debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
    debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
    debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

    debugMessenger = vk::raii::DebugUtilsMessengerEXT(instance, debugUtilsMessengerCreateInfoEXT);
}

void Application::createSurface()
{
    VkSurfaceKHR _surface;
    if (!SDL_Vulkan_CreateSurface(window.get(), *instance, nullptr, &_surface))
        throw std::runtime_error("Couldn't create vulkan surface");
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

void Application::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> devices{instance.enumeratePhysicalDevices()};
    auto devIter{
        std::ranges::find_if(devices, [&](const auto &device) {
            bool supportsVulkan1_3{device.getProperties().apiVersion >= VK_API_VERSION_1_3};
            
        auto queueFamilyProperties{device.getQueueFamilyProperties()};
        bool supportsGraphics{std::ranges::any_of(
            queueFamilyProperties, [](const auto &qfp) { return !!(qfp.queueFlags & vk::QueueFlagBits::eGraphics); })};
        
        auto deviceExtensionProperties{device.enumerateDeviceExtensionProperties()};
        bool supportsRequiredExtensions{
            std::ranges::all_of(
                requiredDeviceExtensions, [&deviceExtensionProperties](const auto &requiredDeviceExtension) {
                    return std::ranges::any_of(
                        deviceExtensionProperties, [requiredDeviceExtension](const auto &deviceExtensionProperty) {
                            return std::strcmp(deviceExtensionProperty.extensionName, requiredDeviceExtension) == 0;
                        });
                })};

        auto features{device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan11Features,
                                                   vk::PhysicalDeviceVulkan13Features,
                                                   vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()};
        bool supportsRequiredFeatures{
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState};
        
        return supportsVulkan1_3 && supportsGraphics && supportsRequiredExtensions && supportsRequiredFeatures;
    })};
    if (devIter == devices.end())
        throw std::runtime_error("Couldn't find suitable GPU");
    physicalDevice = *devIter;
}

void Application::createLogicDevice()
{
    auto queueFamilyProperties{physicalDevice.getQueueFamilyProperties()};
    uint32_t queueIndex = ~0;

    for (uint32_t qfpIndex{}; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            (physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))) {
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0) {
        throw std::runtime_error(std::format("Couldn't find suitable queue family graphyics and present"));
    }

    float queuePriority{0.5f};
    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceVulkan11Features>
        featureChain;
    featureChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
    featureChain.get<vk::PhysicalDeviceVulkan11Features>().shaderDrawParameters = true;
    featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

    vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceQueueCreateInfo.queueCount = 1;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pNext = &featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueIndex, 0);
}

void Application::createSwapChain()
{
    auto surfaceCapabilities{physicalDevice.getSurfaceCapabilitiesKHR(*surface)};
    swapChainSurfaceFormat = chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));
    auto swapChainPresentMode{chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface))};
    swapChainExtent = chooseSwapExtent(surfaceCapabilities, window.get());
    auto swapChainMinImageCount{chooseSwapMinImageCount(surfaceCapabilities)};

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.surface = *surface;
    swapChainCreateInfo.imageFormat = swapChainSurfaceFormat.format;
    swapChainCreateInfo.imageColorSpace = swapChainSurfaceFormat.colorSpace;
    swapChainCreateInfo.presentMode = swapChainPresentMode;
    swapChainCreateInfo.imageExtent = swapChainExtent;
    swapChainCreateInfo.minImageCount = swapChainMinImageCount;

    swapChainCreateInfo.imageArrayLayers = 1;
    swapChainCreateInfo.imageUsage = vk::ImageUsageFlagBits::eColorAttachment;
    swapChainCreateInfo.imageSharingMode = vk::SharingMode::eExclusive;
    swapChainCreateInfo.preTransform = surfaceCapabilities.currentTransform;
    swapChainCreateInfo.compositeAlpha = vk::CompositeAlphaFlagBitsKHR::eOpaque;
    swapChainCreateInfo.clipped = true;

    swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
    swapChainImages = swapChain.getImages();
}

void Application::createImageViews()
{
    swapChainImageViews.clear();
    vk::ImageViewCreateInfo imageViewCreateInfo;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = swapChainSurfaceFormat.format;
    imageViewCreateInfo.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1};

    for (auto &image : swapChainImages) {
        imageViewCreateInfo.image = image;
        vk::raii::ImageView imageView{device, imageViewCreateInfo};
        swapChainImageViews.push_back(std::move(imageView));
    }
}

