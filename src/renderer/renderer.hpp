#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "../application/window.hpp"
#include "buffers/buffer.hpp"
#include "buffers/deviceBuffer.hpp"
#include "buffers/hostBuffer.hpp"
#include "compute.hpp"
#include "device.hpp"
#include "vk_mem_alloc.h"

class Renderer
{
public:
  Renderer(std::string name, Window* window);
  ~Renderer();

  HostBuffer& createHostBuffer(
      std::string name,
      size_t size = 0,
      void* data = nullptr,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

  DeviceBuffer& createDeviceBuffer(
      std::string name,
      size_t size = 0,
      vk::BufferUsageFlags usageFlags = vk::BufferUsageFlagBits::eStorageBuffer,
      VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_AUTO,
      VmaAllocationCreateFlags flags =
          VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT);

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

  VmaAllocator allocator;

  vk::SwapchainKHR swapchain {VK_NULL_HANDLE};
  vk::Format swapchainImageFormat;
  vk::Extent2D swapchainExtent;
  std::vector<vk::Image> images;
  std::vector<vk::ImageView> imagesViews;
  std::unordered_map<std::string, DeviceBuffer> deviceBuffers;
  std::unordered_map<std::string, HostBuffer> hostBuffers;

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
