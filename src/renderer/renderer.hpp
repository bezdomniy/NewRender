#pragma once

#include <string>
#include <vector>

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

  vk::Instance instance {VK_NULL_HANDLE};

  Window* window = nullptr;
  vk::SurfaceKHR surface {VK_NULL_HANDLE};
  vk::PhysicalDeviceMemoryProperties memoryProperties;
  vk::Device device {VK_NULL_HANDLE};
  vk::CommandPool commandPool {VK_NULL_HANDLE};
  std::vector<vk::CommandBuffer> commandBuffers;
  vk::SwapchainKHR swapchain {VK_NULL_HANDLE};
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imagesViews;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger {VK_NULL_HANDLE};
#endif

  void initVulkan();
  void render();
  void cleanup();

  void createInstance();
  void createDevice();
};
