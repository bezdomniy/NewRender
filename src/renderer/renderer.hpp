#pragma once

#include <fmt/core.h>
#include <string>
#include <vulkan/vulkan.hpp>

class Renderer {
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