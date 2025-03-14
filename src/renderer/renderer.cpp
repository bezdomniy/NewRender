#include <cstdlib>
#include <exception>

#include "renderer.hpp"

#include <fmt/base.h>
#include <vulkan/vulkan_core.h>
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
    vk::ApplicationInfo applicationInfo(appName.c_str(),
                                        VK_MAKE_VERSION(0, 0, 1),
                                        "NewEngine",
                                        VK_MAKE_VERSION(0, 0, 1),
                                        VK_API_VERSION_1_4);

    vk::InstanceCreateInfo instanceCreateInfo({}, &applicationInfo);

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
