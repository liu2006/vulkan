#include "application.hpp"

Application::Application() : width{800}, height{600}, title{"vulkan"} 
{
    std::cout << "Created Application" << std::endl;
}

Application::Application(uint32_t width, uint32_t height, const char *title)
    : width{width}, height{height}, title{title}
{
    std::cout << "Created Application" << std::endl;
}

Application::~Application()
{
    std::cout << "Destroyed Application" << std::endl;
}

void Application::run()
{
    initWindow();
    initVulkan();
    mainLoop();
    cleanup();
}

void Application::initWindow()
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Vulkan_LoadLibrary(nullptr);
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
}
static uint32_t chooseSwapMinImageCount(const vk::SurfaceCapabilitiesKHR &capabilities)
{
    uint32_t minImageCount = std::max(3u, capabilities.minImageCount);
    return (capabilities.maxImageCount > 0 && minImageCount > capabilities.maxImageCount) ?
        capabilities.maxImageCount : minImageCount;
}

static vk::Extent2D chooseSwapExtent(const vk::SurfaceCapabilitiesKHR &capabilities, SDL_Window *window)
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int width{}, height{};
    SDL_GetWindowSizeInPixels(window, &width, &height);
    return
    {
        std::clamp<uint32_t>(width, capabilities.minImageExtent.width,
                             capabilities.maxImageExtent.width),
        std::clamp<uint32_t>(height, capabilities.minImageExtent.height,
                             capabilities.maxImageExtent.height)
    };
}
static vk::PresentModeKHR
chooseSwapPresentMode(const std::vector<vk::PresentModeKHR> &availablePresentModes)
{
    assert(std::ranges::any_of(availablePresentModes, [](const auto &presentMode) {
        return presentMode == vk::PresentModeKHR::eFifo;
    }));
    return std::ranges::any_of(
               availablePresentModes,
               [](const auto &presentMode) { return presentMode == vk::PresentModeKHR::eMailbox; })
               ? vk::PresentModeKHR::eMailbox
               : vk::PresentModeKHR::eFifo;
}
static vk::SurfaceFormatKHR
chooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR> &availableFormats)
{
    assert(!availableFormats.empty());
    auto formatIter = std::ranges::find_if(availableFormats, [](const auto &format) {
        return format.format == vk::Format::eB8G8R8A8Srgb &&
            format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
    });
    return (formatIter != availableFormats.end()) ? *formatIter : availableFormats[0];
}

void Application::createSwapChain()
{
    vk::SurfaceCapabilitiesKHR surfaceCapabilities =
        physicalDevice.getSurfaceCapabilitiesKHR(*surface);

    vk::SurfaceFormatKHR swapChainSurfaceFormat =
        chooseSwapSurfaceFormat(physicalDevice.getSurfaceFormatsKHR(*surface));

    vk::PresentModeKHR swapChainPresentMode =
        chooseSwapPresentMode(physicalDevice.getSurfacePresentModesKHR(*surface));

    vk::Extent2D swapChainExtent = chooseSwapExtent(surfaceCapabilities, window);

    uint32_t swapChainMinImageCount = chooseSwapMinImageCount(surfaceCapabilities);

    vk::SwapchainCreateInfoKHR swapChainCreateInfo;
    swapChainCreateInfo.surface = surface;
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


void Application::createLogicDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties =
        physicalDevice.getQueueFamilyProperties();
    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex{}; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            (physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))) {
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0) {
        throw std::runtime_error("Couldn't find graphics and not suitable present");
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featureChain;
    featureChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
    featureChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState =
        true;

    float queuePriority{0.5f};
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo;
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceQueueCreateInfo.queueFamilyIndex = queueIndex;
    deviceQueueCreateInfo.queueCount = 1;

    vk::DeviceCreateInfo deviceCreateInfo;
    deviceCreateInfo.pNext = featureChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = requiredDeviceExtensions.size();

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueIndex, 0);
}
void Application::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    auto devIter = std::ranges::find_if(devices, [&](const auto &device) {
        bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
        auto queueFamilies = device.getQueueFamilyProperties();
        bool supportsGraphics = std::ranges::any_of(queueFamilies, [](const auto &queueFamily) {
            return !!(queueFamily.queueFlags & vk::QueueFlagBits::eGraphics);
        });

        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            requiredDeviceExtensions,
            [&availableDeviceExtensions](const auto &requiredDeviceExtension) {
                return std::ranges::any_of(
                    availableDeviceExtensions,
                    [requiredDeviceExtension](const auto &availableDeviceExtension) {
                        return std::strcmp(availableDeviceExtension.extensionName,
                                           requiredDeviceExtension) == 0;
                    });
            });

        auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2,
                                                     vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>()
                .extendedDynamicState;
        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions &&
               supportsRequiredFeatures;
    });
    if (devIter != devices.end()) {
        physicalDevice = *devIter;
    } else {
        throw std::runtime_error("Couldn't find suitable GPU");
    }
}
                                    
void Application::createSurface()
{
    VkSurfaceKHR _surface;
    if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &_surface))
        std::runtime_error("Couldn't create surface");
    surface = vk::raii::SurfaceKHR(instance, _surface);
}

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(
    vk::DebugUtilsMessageSeverityFlagBitsEXT severity, vk::DebugUtilsMessageTypeFlagsEXT type,
    const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData, void *)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning) {
        std::cout << std::format("Validation layer: {}, message: {}", to_string(type),
                                 pCallbackData->pMessage)
                  << std::endl;
    }
    return vk::False;
}

void Application::setupDebugMessenger()
{
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose};
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{
        vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
        vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
        vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance};
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT;
    debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
    debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
    debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;
    
    debugMessenger = vk::raii::DebugUtilsMessengerEXT(instance, debugUtilsMessengerCreateInfoEXT);
}

void Application::createInstance()
{
    vk::ApplicationInfo appInfo;
    appInfo.apiVersion = vk::ApiVersion14;
    
    std::vector<const char *> requiredLayers{};
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());        
    }
    std::vector<vk::LayerProperties> layerProperties = context.enumerateInstanceLayerProperties();
    for (const auto &requiredLayer : requiredLayers) {
        if (std::ranges::none_of(layerProperties, [requiredLayer](const auto &layerProperty) {
            return std::strcmp(layerProperty.layerName, requiredLayer) == 0;
        }))
            throw std::runtime_error(std::format("Required layer not supported: {}", requiredLayer));
    }
    std::vector<const char *> requiredExtensions = getRequiredExtensions();
    std::vector<vk::ExtensionProperties> extensionProperties = context.enumerateInstanceExtensionProperties();
    for (const auto &requiredExtension : requiredExtensions) {
        if (std::ranges::none_of(extensionProperties, [requiredExtension](const auto &extensionProperty) {
            return std::strcmp(extensionProperty.extensionName, requiredExtension) == 0;
        }))
            throw std::runtime_error(std::format("Required extension not supported: {}", requiredExtension));
    }
    
    vk::InstanceCreateInfo createInfo;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledLayerCount = requiredLayers.size();
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.enabledExtensionCount = requiredExtensions.size();
    instance = vk::raii::Instance(context, createInfo);
}

std::vector<const char *> Application::getRequiredExtensions()
{
    uint32_t sdlExtensionCount{};
    auto sdlExtensions = SDL_Vulkan_GetInstanceExtensions(&sdlExtensionCount);
    std::vector extensions(sdlExtensions, sdlExtensions + sdlExtensionCount);
    if (enableValidationLayers) {
        extensions.push_back(vk::EXTDebugUtilsExtensionName);
    }
    return extensions;
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
    
};
void Application::cleanup()
{
    SDL_DestroyWindow(window);
    SDL_Vulkan_UnloadLibrary();
    SDL_Quit();
};

