
#include <cstdlib>
#include <exception>

#include "renderer.hpp"

#include <fmt/base.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_structs.hpp>

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
  instance = createInstance();
}

void Renderer::render() {}

void Renderer::cleanup() {}

auto Renderer::createInstance() -> vk::Instance
{
  try {
    std::vector<const char*> extNames;
    extNames.push_back(VK_KHR_GET_PHYSICAL_DEVICE_PROPERTIES_2_EXTENSION_NAME);
    extNames.push_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);

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
        static_cast<uint32_t>(extNames.size()),
        extNames.data());

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
