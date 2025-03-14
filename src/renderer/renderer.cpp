#include "renderer.hpp"
#include <vulkan/vulkan_core.h>

Renderer::Renderer(std::string name) : appName(name) {};

Renderer::~Renderer() {}

void Renderer::run() {
  //   initVulkan();
  fmt::print("Hello {}!\n", appName);
}

void Renderer::initVulkan() { instance = createInstance(); }

void Renderer::render() {}

void Renderer::cleanup() {}

vk::Instance Renderer::createInstance() {
  vk::ApplicationInfo applicationInfo(appName.c_str(), VK_MAKE_VERSION(0, 0, 1),
                                      "NewEngine", VK_MAKE_VERSION(0, 0, 1),
                                      VK_API_VERSION_1_4);
}
