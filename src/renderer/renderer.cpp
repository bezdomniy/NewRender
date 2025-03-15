#include "renderer.hpp"

#include <fmt/base.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "validation.hpp"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

Renderer::Renderer(std::string name)
    : appName(name) {};

Renderer::~Renderer()
{
  cleanup();
}

void Renderer::run()
{
  initVulkan();
  fmt::print("Hello {}!\n", appName);
}

void Renderer::initVulkan()
{
  createDevice();
#if !defined(NDEBUG)
  debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
      makeDebugUtilsMessengerCreateInfoEXT());
#endif
}

void Renderer::render() {}

void Renderer::cleanup()
{
  device.destroy();
#if !defined(NDEBUG)
  instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
  instance.destroy();
}

void Renderer::createDevice()
{
  try {
    std::vector<const char*> enabledExtensions;

#if !defined(NDEBUG)
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif

    std::vector<const char*> enabledLayers;
#if !defined(NDEBUG)
    enabledLayers.push_back("VK_LAYER_KHRONOS_validation");
#endif

    vk::ApplicationInfo applicationInfo(appName.c_str(),
                                        VK_MAKE_VERSION(0, 0, 1),
                                        "NewEngine",
                                        VK_MAKE_VERSION(0, 0, 1),
                                        VK_API_VERSION_1_4);

#ifdef __APPLE__
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR};

    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
#else
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {};
#endif

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // initialize with explicitly providing a DynamicLoader
    vk::detail::DynamicLoader dl;
    VULKAN_HPP_DEFAULT_DISPATCHER.init(dl);
#endif

    vk::InstanceCreateInfo instanceCreateInfo(
        flags,
        &applicationInfo,
        static_cast<uint32_t>(enabledLayers.size()),
        enabledLayers.data(),
        static_cast<uint32_t>(enabledExtensions.size()),
        enabledExtensions.data());

    instance = vk::createInstance(instanceCreateInfo);

#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
    // initialize minimal set of function pointers
    VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif

    auto physicalDevice = instance.enumeratePhysicalDevices().front();
    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    // get the first index into queueFamiliyProperties which supports graphics
    auto propertyIterator =
        std::find_if(queueFamilyProperties.begin(),
                     queueFamilyProperties.end(),
                     [](vk::QueueFamilyProperties const& qfp)
                     { return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });
    auto graphicsQueueFamilyIndex =
        std::distance(queueFamilyProperties.begin(), propertyIterator);
    assert(graphicsQueueFamilyIndex < queueFamilyProperties.size());

    // create a Device
    float queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(
        vk::DeviceQueueCreateFlags(),
        static_cast<uint32_t>(graphicsQueueFamilyIndex),
        1,
        &queuePriority);

    std::vector<const char*> enabledDeviceExtensions;
#ifdef __APPLE__
    #ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
    #define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
    #endif

    enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

    vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
                                          deviceQueueCreateInfo,
                                          {},
                                          enabledDeviceExtensions
                                        );

    device = physicalDevice.createDevice(deviceCreateInfo);

  } catch (vk::SystemError& err) {
    fmt::println("vk::SystemError: {}", err.what());
    exit(-1);
  } catch (std::exception& err) {
    fmt::println("std::exception: {}", err.what());
    exit(-1);
  } catch (...) {
    fmt::println("unknown error");
    exit(-1);
  }
}
