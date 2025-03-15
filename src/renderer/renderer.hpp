#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

class Renderer
{
public:
  Renderer(std::string name);
  ~Renderer();

  void run();

private:
  std::string appName;

  vk::Instance instance;
  vk::Device device;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
#endif

  void initVulkan();
  void render();
  void cleanup();

  void createDevice();
};
