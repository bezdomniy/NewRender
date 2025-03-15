#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#include "../application/window.hpp"

class Renderer
{
public:
  Renderer(std::string name, Window* window);
  ~Renderer();

  void run();

private:
  std::string appName;

  vk::Instance instance;
  Window* window;
  vk::SurfaceKHR surface;
  vk::Device device;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
#endif

  void initVulkan();
  void render();
  void cleanup();

  void createDevice();
};
