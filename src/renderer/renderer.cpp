#include "renderer.hpp"

#include <fmt/base.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

#include "validation.hpp"
#include "vulkan/vulkan_hpp_macros.hpp"

#if VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1
VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE
#endif

Renderer::Renderer(std::string name)
    : appName(name) {};

Renderer::~Renderer() {}

void Renderer::run()
{
  initVulkan();
  fmt::print("Hello {}!\n", appName);
}

void Renderer::initVulkan()
{
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize minimal set of function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
#endif
  instance = createInstance();
#if (VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1)
  // initialize minimal set of function pointers
  VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);
#endif
#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger =
      instance.createDebugUtilsMessengerEXT(
          makeDebugUtilsMessengerCreateInfoEXT());
#endif
}

void Renderer::render() {}

void Renderer::cleanup() {}

auto Renderer::createInstance() -> vk::Instance
{
  try {
    std::vector<const char*> enabledExtensions;
    enabledExtensions.push_back(
        VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    enabledExtensions.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

#if !defined(NDEBUG)
    enabledExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

#endif

    vk::ApplicationInfo applicationInfo(appName.c_str(),
                                        VK_MAKE_VERSION(0, 0, 1),
                                        "NewEngine",
                                        VK_MAKE_VERSION(0, 0, 1),
                                        VK_API_VERSION_1_4);

#ifdef __APPLE__
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {
        vk::InstanceCreateFlagBits::eEnumeratePortabilityKHR};
#else
    vk::InstanceCreateFlags flags = vk::InstanceCreateFlags {};
#endif

    vk::InstanceCreateInfo instanceCreateInfo(
        flags,
        &applicationInfo,
        0,
        nullptr,
        static_cast<uint32_t>(enabledExtensions.size()),
        enabledExtensions.data());

    return vk::createInstance(instanceCreateInfo);
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
