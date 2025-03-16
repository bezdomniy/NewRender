#include <cstdint>
#include <set>
#include <vector>

#include "renderer.hpp"

#include <fmt/base.h>

#include "validation.hpp"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

Renderer::Renderer(std::string name, Window* window)
    : window(window)
    , appName(name) {};

Renderer::~Renderer()
{
  cleanup();
}

void Renderer::run()
{
  initVulkan();
  fmt::print("Hello {}!\n", appName);
}

void Renderer::createBuffer(std::string name,
                            void* data,
                            size_t size,
                            vk::BufferUsageFlags usageFlags,
                            vk::MemoryPropertyFlags memoryFlags) {
  buffers.try_emplace(name, device, memoryProperties, data, size, usageFlags, memoryFlags);
};

void Renderer::initVulkan()
{
  createInstance();
  surface = window->create_surface(instance, physicalDevice);

  createDevice();
}

void Renderer::render() {}

void Renderer::cleanup()
{
  buffers.clear();
  device.destroyCommandPool(commandPool);
  for (auto& imageView : imagesViews) {
    device.destroyImageView(imageView);
  }
  device.destroySwapchainKHR(swapchain);
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
  std::vector<const char*> enabledDeviceExtensions {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  pickPhysicalDevice(enabledDeviceExtensions);

  queueFamilyIndices = findQueueFamilies(physicalDevice);

  memoryProperties = physicalDevice.getMemoryProperties();

  // create a Device
  float queuePriority = 0.0f;
  vk::DeviceQueueCreateInfo deviceQueueCreateInfo(vk::DeviceQueueCreateFlags(),
                                                  queueFamilyIndices.graphicsFamily.value(),
                                                  1,
                                                  &queuePriority);


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

Renderer::QueueFamilyIndices Renderer::findQueueFamilies(vk::PhysicalDevice device) {
  QueueFamilyIndices indices;

  uint32_t queueFamilyCount = 0;
  device.getQueueFamilyProperties(&queueFamilyCount, nullptr);

  std::vector<vk::QueueFamilyProperties> queueFamilies(queueFamilyCount);
  device.getQueueFamilyProperties(&queueFamilyCount, queueFamilies.data());

  int i = 0;
  for (const auto& queueFamily : queueFamilies) {
      if (queueFamily.queueFlags & vk::QueueFlagBits::eGraphics) {
          indices.graphicsFamily = i;
      }

      VkBool32 presentSupport = false;
      auto result = device.getSurfaceSupportKHR(i, surface, &presentSupport);

      if (result != vk::Result::eSuccess) {
        throw std::runtime_error("Failed to create window surface.");
      }

      if (presentSupport) {
          indices.presentFamily = i;
      }

      if (indices.isComplete()) {
          break;
      }

      i++;
  }

  return indices;
}


void Renderer::pickPhysicalDevice(std::vector<const char*>& deviceExtensions) {
  uint32_t deviceCount = 0;
  (void) instance.enumeratePhysicalDevices(&deviceCount, nullptr);

  if (deviceCount == 0) {
      throw std::runtime_error("failed to find GPUs with Vulkan support!");
  }

  std::vector<vk::PhysicalDevice> devices(deviceCount);
  (void) instance.enumeratePhysicalDevices(&deviceCount, devices.data());

  for (const auto& device : devices) {

    QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device, deviceExtensions);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    auto deviceSuitable = indices.isComplete() && extensionsSupported && swapChainAdequate;
      if (deviceSuitable) {
          physicalDevice = device;
          break;
      }
  }

  if (physicalDevice == VK_NULL_HANDLE) {
      throw std::runtime_error("failed to find a suitable GPU!");
  }
}


Renderer::SwapChainSupportDetails Renderer::querySwapChainSupport(vk::PhysicalDevice device) {
  SwapChainSupportDetails details;

  (void) device.getSurfaceCapabilitiesKHR(surface, &details.capabilities);

  uint32_t formatCount;
  (void) device.getSurfaceFormatsKHR(surface, &formatCount, nullptr);

  if (formatCount != 0) {
      details.formats.resize(formatCount);
      (void) device.getSurfaceFormatsKHR(surface, &formatCount, details.formats.data());
  }

  uint32_t presentModeCount;
  (void) device.getSurfacePresentModesKHR(surface, &presentModeCount, nullptr);

  if (presentModeCount != 0) {
      details.presentModes.resize(presentModeCount);
      (void) device.getSurfacePresentModesKHR(surface, &presentModeCount, details.presentModes.data());
  }

  return details;
}


bool Renderer::checkDeviceExtensionSupport(vk::PhysicalDevice device, const std::vector<const char*>& deviceExtensions) {
  uint32_t extensionCount;
  (void) device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, nullptr);

  std::vector<vk::ExtensionProperties> availableExtensions(extensionCount);
  (void) device.enumerateDeviceExtensionProperties(nullptr, &extensionCount, availableExtensions.data());

  std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

  for (const auto& extension : availableExtensions) {
      requiredExtensions.erase(extension.extensionName);
  }

  return requiredExtensions.empty();
}