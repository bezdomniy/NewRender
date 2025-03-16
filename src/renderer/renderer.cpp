#include <cstdint>
#include <vector>

#include "renderer.hpp"

#include <fmt/base.h>

#include "buffer.hpp"
#include "validation.hpp"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

Renderer::Renderer(std::string name, Window* window)
    : window(window)
    , appName(name) {};

Renderer::~Renderer()
{
  // cleanup();
}

void Renderer::run()
{
  initVulkan();

  std::vector<uint32_t> numbers {1, 2, 3};

  Buffer testBuf(device,
                 memoryProperties,
                 numbers.data(),
                 numbers.size() * sizeof(uint32_t),
                 vk::BufferUsageFlagBits::eUniformBuffer,
                 vk::MemoryPropertyFlagBits::eHostVisible
                     | vk::MemoryPropertyFlagBits::eHostCoherent);

  fmt::print("Hello {}!\n", appName);
}

void Renderer::createBuffer(std::string name,
                            void* data,
                            size_t size,
                            vk::BufferUsageFlags usageFlags,
                            vk::MemoryPropertyFlags memoryFlags) {
  // TODO
};

void Renderer::initVulkan()
{
  createInstance();

  createDevice();
}

void Renderer::render() {}

void Renderer::cleanup()
{
  device.destroy();
#if !defined(NDEBUG)
  instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
  vkDestroySurfaceKHR(instance, surface, nullptr);
  instance.destroy();
}

void Renderer::createInstance()
{
  try {
    uint32_t glfwExtensionCount = 0;
    auto glfwExtensions = window->getExtensions(&glfwExtensionCount);

    std::vector<const char*> enabledExtensions(
        glfwExtensions, glfwExtensions + glfwExtensionCount);

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

#if !defined(NDEBUG)
    debugUtilsMessenger = instance.createDebugUtilsMessengerEXT(
        makeDebugUtilsMessengerCreateInfoEXT());
#endif

  } catch (vk::SystemError& err) {
    fmt::println("vk::SystemError: {}", err.what());
    exit(-1);
  }

  catch (std::exception& err)
  {
    fmt::println("std::exception: {}", err.what());
    exit(-1);
  }

  catch (...)
  {
    fmt::println("unknown error");
    exit(-1);
  }
}

void Renderer::createDevice()
{
  auto physicalDevices = instance.enumeratePhysicalDevices();
  vk::PhysicalDevice physicalDevice;

  uint32_t graphicsQueueFamilyIndex = 0;
  bool foundGraphicsQueue = false;
  for (size_t i = 0; i < physicalDevices.size() && !foundGraphicsQueue; i++) {
    physicalDevice = physicalDevices[i];

    auto queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    if (queueFamilyProperties.empty()) {
      throw std::runtime_error("No queue family found.");
    }

    if (surface) {
      instance.destroySurfaceKHR(surface);
    }

    surface = window->create_surface(instance, physicalDevice);

    // get the first index into queueFamiliyProperties which supports graphics
    for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
      vk::Bool32 supports_present =
          physicalDevice.getSurfaceSupportKHR(i, surface);
      if ((queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
          && supports_present)
      {
        graphicsQueueFamilyIndex = i;
        foundGraphicsQueue = true;
        break;
      }
    }
  }

  memoryProperties = physicalDevice.getMemoryProperties();
  if (!foundGraphicsQueue) {
    throw std::runtime_error(
          "Did not find suitable queue which supports graphics and "
          "presentation.");
  }

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(),
                                                  graphicsQueueFamilyIndex,
                                                  1,
                                                  &queuePriority);

  std::vector<const char*> enabledDeviceExtensions {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef __APPLE__
#  ifndef VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME
#    define VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME "VK_KHR_portability_subset"
#  endif

  enabledDeviceExtensions.push_back(VK_KHR_PORTABILITY_SUBSET_EXTENSION_NAME);
#endif

  vk::DeviceCreateInfo deviceCreateInfo(vk::DeviceCreateFlags(),
                                        deviceQueueCreateInfo,
                                        {},
                                        enabledDeviceExtensions);

  device = physicalDevice.createDevice(deviceCreateInfo);
}
