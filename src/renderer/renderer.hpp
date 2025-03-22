#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "../application/window.hpp"
#include "buffer.hpp"
#include "compute.hpp"
#include "device.hpp"

class Renderer
{
public:
  Renderer(std::string name, Window* window);
  ~Renderer();

  void createBuffer(std::string name,
                    vk::BufferUsageFlags usageFlags,
                    vk::MemoryPropertyFlags memoryFlags,
                    size_t size,
                    void* data = nullptr);

  Buffer createBuffer(vk::BufferUsageFlags usageFlags,
                      vk::MemoryPropertyFlags memoryFlags,
                      size_t size,
                      void* data = nullptr);

  // void createComputeTask(std::string name);
  void run();

private:
  std::string appName;

  vk::Instance instance {VK_NULL_HANDLE};

  Window* window = nullptr;
  vk::SurfaceKHR surface {VK_NULL_HANDLE};
  vk::DescriptorPool descriptorPool {VK_NULL_HANDLE};
  // vk::CommandPool commandPool {VK_NULL_HANDLE};
  // std::vector<vk::CommandBuffer> commandBuffers;
  vk::SwapchainKHR swapchain {VK_NULL_HANDLE};
  vk::Format swapchainImageFormat;
  vk::Extent2D swapchainExtent;
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imagesViews;
  std::unordered_map<std::string, Buffer> buffers;

  std::unique_ptr<Device> device = nullptr;
  std::unique_ptr<Compute> compute = nullptr;

#if !defined(NDEBUG)
  vk::DebugUtilsMessengerEXT debugUtilsMessenger {VK_NULL_HANDLE};
#endif

  void initVulkan();
  void render();
  void cleanup();

  void createInstance();
};
