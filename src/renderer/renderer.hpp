#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

class Renderer
{
public:
  Renderer(std::string name);
  ~Renderer();

  void run();

private:
  std::string appName;

  vk::Instance instance;

  void initVulkan();
  void render();
  void cleanup();

  vk::Instance createInstance();
};
