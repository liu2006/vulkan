#include "sdlException.hpp"
#include "application.hpp"

static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void *)
{
    if (severity >= vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning)
        std::cerr << std::format("Validation type: {}, msg: {}", to_string(type), pCallbackData->pMessage);
    return vk::False;
}

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

void Application::initVulkan()
{
    createInstance();
    setupDebugMessenger();
    createSurface();
    pickPhysicalDevice();
    createLogicDevice();
}
void Application::createSurface()
{
    VkSurfaceKHR _surface{};
    if (!SDL_Vulkan_CreateSurface(window, *instance, nullptr, &_surface))
        throw std::runtime_error("Couldn't create surface");
    surface = vk::raii::SurfaceKHR(instance, _surface);
    
}
void Application::createLogicDevice()
{
    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    uint32_t queueIndex = ~0;
    for (uint32_t qfpIndex{}; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
        if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
            physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
            queueIndex = qfpIndex;
            break;
        }
    }
    if (queueIndex == ~0) {
        throw std::runtime_error("Couldn't find queue Graphics/Persent");
    }

    vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                       vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>
        featuresChain{};
    featuresChain.get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering = true;
    featuresChain.get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState = true;

    float queuePriority{0.5f};
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo{};
    deviceQueueCreateInfo.pQueuePriorities = &queuePriority;
    deviceQueueCreateInfo.queueCount = 1;
    deviceQueueCreateInfo.queueFamilyIndex = queueIndex;

    vk::DeviceCreateInfo deviceCreateInfo{};
    deviceCreateInfo.pNext = featuresChain.get<vk::PhysicalDeviceFeatures2>();
    deviceCreateInfo.pQueueCreateInfos = &deviceQueueCreateInfo;
    deviceCreateInfo.queueCreateInfoCount = 1;
    deviceCreateInfo.ppEnabledExtensionNames = requiredDeviceExtensions.data();
    deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size());

    device = vk::raii::Device(physicalDevice, deviceCreateInfo);
    queue = vk::raii::Queue(device, queueIndex, 0);
}
void Application::pickPhysicalDevice()
{
    std::vector<vk::raii::PhysicalDevice> devices = instance.enumeratePhysicalDevices();
    auto devIter = std::ranges::find_if(devices, [&](const auto &device) {
        bool supportsVulkan1_3 = device.getProperties().apiVersion >= VK_API_VERSION_1_3;
        auto queueFamilyProperties = device.getQueueFamilyProperties();

        bool supportsGraphics = std::ranges::any_of(queueFamilyProperties, [](const auto &queueFamilyProperty) {
            return (queueFamilyProperty.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlags>(0);
        });

        auto availableDeviceExtensions = device.enumerateDeviceExtensionProperties();
        bool supportsAllRequiredExtensions = std::ranges::all_of(
            requiredDeviceExtensions, [&availableDeviceExtensions](const auto &requiredDeviceExtension) {
                return std::ranges::any_of(availableDeviceExtensions, [requiredDeviceExtension](const auto &availableDeviceExtension) {
                    return std::strcmp(availableDeviceExtension.extensionName, requiredDeviceExtension) == 0;
                });
            });
        auto features = device.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features,
                                                     vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
        bool supportsRequiredFeatures =
            features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
            features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
        
        return supportsVulkan1_3 && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
    });
    if (devIter == devices.end()) {
        throw std::runtime_error("Couldn't find suitable GPU");
    } else {
        physicalDevice = *devIter;
    }
}
void Application::setupDebugMessenger()
{
    vk::DebugUtilsMessageSeverityFlagsEXT severityFlags{vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eError |
                                                        vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose};
    vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags{vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                                                       vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance};
    vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{};
    debugUtilsMessengerCreateInfoEXT.messageSeverity = severityFlags;
    debugUtilsMessengerCreateInfoEXT.messageType = messageTypeFlags;
    debugUtilsMessengerCreateInfoEXT.pfnUserCallback = &debugCallback;

    debugMessenger = instance.createDebugUtilsMessengerEXT(debugUtilsMessengerCreateInfoEXT);
}
void Application::createInstance()
{
    vk::ApplicationInfo appInfo{};
    appInfo.pApplicationName = title;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No engine";
    appInfo.apiVersion = vk::ApiVersion14;
    
    std::vector<const char *> requiredLayers{};
    if (enableValidationLayers) {
        requiredLayers.assign(validationLayers.begin(), validationLayers.end());
    }
    auto layerProperties = context.enumerateInstanceLayerProperties();
    for (const auto &requiredLayer : requiredLayers) {
        if (std::ranges::none_of(layerProperties, [requiredLayer](const auto &layerProperty) {
            return std::strcmp(layerProperty.layerName, requiredLayer);
        }))
            throw std::runtime_error(std::format("Required layer not supports: {}", requiredLayer));
    }
    auto requiredExtensions = getRequiredExtensions();
    auto extensionProperties = context.enumerateInstanceExtensionProperties();
    for (const auto &requiredExtension : requiredExtensions) {
        if (std::ranges::none_of(extensionProperties, [requiredExtension](const auto &extensionProperty) {
            return std::strcmp(extensionProperty.extensionName, requiredExtension) == 0; 
        }))
            throw std::runtime_error(std::format("Required extension not supported: {}", requiredExtension));

    }
    vk::InstanceCreateInfo createInfo{};
    createInfo.pApplicationInfo = &appInfo;
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();
    createInfo.ppEnabledLayerNames = requiredLayers.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.enabledLayerCount = static_cast<uint32_t>(requiredLayers.size());
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

